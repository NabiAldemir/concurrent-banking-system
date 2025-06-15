#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_ACCOUNTS 5
#define MAX_TRANSACTIONS 100

typedef struct {
    int account_id;
    int balance;
} Account;

typedef struct {
    char type[10]; // "deposit", "withdraw", "transfer"
    int from;
    int to;
    int amount;
    int status; // 0 = pending, 1 = success, -1 = failed
} Transaction;

// Global semaphore (named)
sem_t *sem;

	int main() {
    // Paylaşımlı bellek ID'leri
    int shm_account_id;
    int shm_transaction_id;

    // Paylaşımlı bellek göstericileri
    Account *accounts;
    Transaction *transactions;

    // Paylaşımlı bellek oluştur (hesaplar)
    shm_account_id = shmget(IPC_PRIVATE, sizeof(Account) * MAX_ACCOUNTS, IPC_CREAT | 0666);
    if (shm_account_id < 0) {
        perror("shmget error (accounts)");
        exit(1);
    }
    accounts = (Account *)shmat(shm_account_id, NULL, 0);

    // Paylaşımlı bellek oluştur (işlemler)
    shm_transaction_id = shmget(IPC_PRIVATE, sizeof(Transaction) * MAX_TRANSACTIONS, IPC_CREAT | 0666);
    if (shm_transaction_id < 0) {
        perror("shmget error (transactions)");
        exit(1);
    }
    transactions = (Transaction *)shmat(shm_transaction_id, NULL, 0);

    // Semafor oluştur (named)
    sem = sem_open("/bank_sem", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Örnek başlangıç hesap verisi
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = 500; // her hesabın başlangıç bakiyesi 500
    }
FILE *fp = fopen("transactions.txt", "r");
    if (!fp) {
        perror("transactions.txt açılırken hata");
        exit(1);
    }

    int t_count = 0;
    while (fscanf(fp, "%s %d %d %d", transactions[t_count].type,
                  &transactions[t_count].from,
                  &transactions[t_count].to,
                  &transactions[t_count].amount) == 4) {
        transactions[t_count].status = 0; // pending
        t_count++;
    }
    fclose(fp);

    // Her işlem için bir child process oluştur
    for (int i = 0; i < t_count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // CHILD PROCESS

            sem_wait(sem); // 🔐 KAYNAĞI KİLİTLE

            int from = transactions[i].from;
            int to = transactions[i].to;
            int amt = transactions[i].amount;
            char *type = transactions[i].type;

            int success = 0;

            if (strcmp(type, "deposit") == 0) {
                accounts[to].balance += amt;
                success = 1;
            } else if (strcmp(type, "withdraw") == 0) {
                if (accounts[from].balance >= amt) {
                    accounts[from].balance -= amt;
                    success = 1;
                }
            } else if (strcmp(type, "transfer") == 0) {
                int first = from < to ? from : to;
                int second = from > to ? from : to;

                if (accounts[from].balance >= amt) {
                    accounts[from].balance -= amt;
                    accounts[to].balance += amt;
                    success = 1;
                }
            }

            transactions[i].status = success ? 1 : -1;

            sem_post(sem); // 🔓 KAYNAĞI SERBEST BIRAK

            exit(success ? 0 : -1);
        }
    }

    // PARENT PROCESS: çocukları bekle ve gerekirse retry et
    for (int i = 0; i < t_count; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 255) {
            // başarısız ise tekrar dene
            printf("Transaction %d başarısız, tekrar deneniyor...\n", i);
            pid_t retry_pid = fork();
            if (retry_pid == 0) {
                sem_wait(sem);
                int from = transactions[i].from;
                int to = transactions[i].to;
                int amt = transactions[i].amount;
                char *type = transactions[i].type;

                int success = 0;

                if (strcmp(type, "deposit") == 0) {
                    accounts[to].balance += amt;
                    success = 1;
                } else if (strcmp(type, "withdraw") == 0) {
                    if (accounts[from].balance >= amt) {
                        accounts[from].balance -= amt;
                        success = 1;
                    }
                } else if (strcmp(type, "transfer") == 0) {
                    if (accounts[from].balance >= amt) {
                        accounts[from].balance -= amt;
                        accounts[to].balance += amt;
                        success = 1;
                    }
                }

                transactions[i].status = success ? 1 : -1;
                sem_post(sem);
                exit(success ? 0 : -1);
            } else {
                wait(NULL);
            }
        }
    }

    // Sonuçları yazdır
    printf("\nFinal account balances:\n");
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        printf("Account %d: %d\n", accounts[i].account_id, accounts[i].balance);
    }

    printf("\nTransaction Log:\n");
    for (int i = 0; i < t_count; i++) {
        printf("Transaction %d: %s %d -> %d Amount: %d (%s)\n", i,
               transactions[i].type,
               transactions[i].from,
               transactions[i].to,
               transactions[i].amount,
               transactions[i].status == 1 ? "Success" :
               transactions[i].status == -1 ? "Failed" : "Pending");
    }

    // Temizlik
    sem_close(sem);
    sem_unlink("/bank_sem");
    shmdt(accounts);
    shmdt(transactions);
    shmctl(shm_account_id, IPC_RMID, NULL);
    shmctl(shm_transaction_id, IPC_RMID, NULL);

}

