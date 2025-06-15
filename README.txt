Operating Systems Assignment 2: Concurrent Banking System
--------------------------------------------------------

Prepared by: İbrahim Nabi ALDEMİR
Student ID: 212010020007
Course:: Operating Systems
Instructor: Reza Zare HASSANPOUR

Brief Description
This project implements a concurrent banking system using shared memory and semaphores. Each transaction is executed as a separate child process. To prevent deadlocks in transfer operations, the process always accesses the account with the smaller ID first. Failed transactions are retried once.

Usage;
1. WSL/Ubuntu:
   gcc main.c -o bank -lrt -pthread
   ./bank

2. Write transaction operations into the transactions.txt file.

3. After running the program, results can be viewed in the terminal or optionally written to output.txt.

Project Files;
- main.c              → Source code
- transactions.txt    → Input data (transaction list)
- output.txt          → Sample output
- README.txt          → Project description
