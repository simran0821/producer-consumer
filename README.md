# Producer–Consumer Project (C, Linux)
This project is my implementation of the **Producer–Consumer problem** using **C language** on **Linux (Ubuntu/WSL) **.  
It uses **shared memory** so the producer and consumer processes can share data, and **semaphores** for synchronization and mutual exclusion.
The producer creates items and puts them into a shared buffer (the "table"), and the consumer removes them.  
The table can hold **only 2 items** at a time.  
If the table is full, the producer waits.  
If the table is empty, the consumer waits.  
Both programs run as **separate processes**, not threads.

 How to Compile
Make sure you are inside the `producer-consumer` folder, then run:
make
How to Run
Run both producer and consumer together (they will produce and consume 20 items):
make run-demo
You can also run them manually:
./bin/producer &
./bin/consumer &
To remove shared memory and semaphores (cleanup):
./bin/cleanup
How It Works
Shared Memory (shm_open, ftruncate, mmap)
Stores the shared buffer, which both processes use.

Semaphores (sem_open, sem_wait, sem_post)
Used to control access and make sure producer and consumer don’t conflict.

Mutual Exclusion
A separate semaphore mutex is used so only one process accesses the buffer at a time.

Files Included
File	Description
src/producer.c =	Producer process
src/consumer.c	 =Consumer process
src/common.h =	Shared constants and structure
src/cleanup.c =Removes shared memory and semaphores
Makefile =	Compiles all programs
example_output/ =	Contains screenshots of program run

Example Results
Compilation:

Running Producer & Consumer:

Cleanup:

Author
Simran Gautam
Operating Systems — Assignment #1
Fall 2025
