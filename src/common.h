#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>
#include <fcntl.h>      // O_* constants
#include <sys/mman.h>   // shm, mmap
#include <sys/stat.h>   // mode constants
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#define SHM_NAME        "/pc_shm_table"
#define SEM_EMPTY_NAME  "/pc_sem_empty"
#define SEM_FULL_NAME   "/pc_sem_full"
#define SEM_MUTEX_NAME  "/pc_sem_mutex"

#define CAPACITY 2  // table can hold exactly 2 items

typedef struct {
    int buffer[CAPACITY];
    int in;     // next write index
    int out;    // next read index
    int count;  // current number of items
} shm_table_t;

#endif
