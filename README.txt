# Concurrent Banking System 🏦

This project is an Operating Systems assignment aimed at implementing a **concurrent banking system** using **C programming language**, **processes**, **shared memory**, **semaphores**, and **inter-process communication (IPC)** techniques.

## 📌 Project Overview

The system simulates a bank with multiple accounts where transactions like deposits, withdrawals, and transfers are handled **concurrently** by creating **child processes**. Shared memory is used to store the account balances and transaction logs, and synchronization is ensured using **POSIX semaphores**.

### 🔧 Features
- Shared memory for storing account balances and transaction logs
- Process creation for executing each transaction
- Semaphore-based synchronization to prevent race conditions
- Transfer deadlock prevention by locking smaller account ID first
- Transaction retry mechanism for failed operations (e.g., insufficient funds)

---

## 📁 Project Structure

bank_project/
├── main.c // Main source code
├── transactions.txt // List of transaction requests
├── output.txt // Sample program output
├── bank
└── README.md // This file

---

## ⚙️ How It Works

1. The program reads accounts and initial balances.
2. Then it reads transactions from `transactions.txt`, e.g.:
deposit -1 0 100
withdraw 1 -1 50
transfer 2 3 615

3. Each transaction is executed by a separate child process.
4. If a transfer or withdrawal fails, the parent process retries the transaction once.
5. Final account balances and a transaction log are displayed.

---

## ▶️ How to Compile & Run

Make sure you are using **Linux** or **WSL** with a C compiler.

```bash
# Compile
gcc main.c -o bank -lrt -pthread

# Run
./bank
📌 Sample Output
Final account balances:
Account 0: 630
Account 1: 510
Account 2: 5
Account 3: 1055
Account 4: 610

Transaction Log:
Transaction 0: Deposit 100 to Account 0 (Success)
Transaction 1: Withdraw 50 from Account 1 (Success)
Transaction 2: Transfer 615 from Account 2 to Account 3 (Failed)
Transaction 3: Deposit 200 to Account 4 (Success)
Transaction 4: Transfer 30 from Account 1 to Account 0 (Success)
Transaction 5: Withdraw 60 from Account 3 (Success)
Transaction 6: Deposit 120 to Account 2 (Success)
Transaction 7: Transfer 90 from Account 4 to Account 1 (Success)
Transaction 2: Transfer 615 from Account 2 to Account 3 (Success)
