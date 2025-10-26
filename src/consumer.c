#include "common.h"

static volatile sig_atomic_t keep_running = 1;
static void on_sigint(int sig) { (void)sig; keep_running = 0; }

static int retry_open_shm(int *fd_out, shm_table_t **tbl_out) {
    // Try for ~5 seconds waiting for producer to create first
    for (int i = 0; i < 50; ++i) {
        int fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (fd != -1) {
            shm_table_t *tbl = mmap(NULL, sizeof(shm_table_t),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (tbl != MAP_FAILED) {
                *fd_out = fd;
                *tbl_out = tbl;
                return 0;
            }
            close(fd);
        }
        struct timespec ts = {0, 100 * 1000000L}; // 100ms
        nanosleep(&ts, NULL);
    }
    return -1;
}

static sem_t* retry_open_sem(const char *name) {
    for (int i = 0; i < 50; ++i) {
        sem_t *s = sem_open(name, 0);
        if (s != SEM_FAILED) return s;
        struct timespec ts = {0, 100 * 1000000L};
        nanosleep(&ts, NULL);
    }
    return SEM_FAILED;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, on_sigint);

    long target = 0;
    if (argc >= 2) {
        target = strtol(argv[1], NULL, 10);
        if (target < 0) target = 0;
    }

    int shm_fd = -1;
    shm_table_t *tbl = NULL;
    if (retry_open_shm(&shm_fd, &tbl) != 0) {
        fprintf(stderr, "[consumer %d] failed to open shared memory (is producer running?)\n", getpid());
        return 1;
    }

    sem_t *sem_empty = retry_open_sem(SEM_EMPTY_NAME);
    if (sem_empty == SEM_FAILED) { perror("consumer: sem_open empty"); return 1; }
    sem_t *sem_full = retry_open_sem(SEM_FULL_NAME);
    if (sem_full == SEM_FAILED) { perror("consumer: sem_open full"); return 1; }
    sem_t *sem_mutex = retry_open_sem(SEM_MUTEX_NAME);
    if (sem_mutex == SEM_FAILED) { perror("consumer: sem_open mutex"); return 1; }

    printf("[consumer %d] started (target=%ld)\n", getpid(), target);
    fflush(stdout);

    long consumed = 0;
    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    while (keep_running && (target == 0 || consumed < target)) {
        if (sem_wait(sem_full) == -1 && errno == EINTR) continue;
        if (sem_wait(sem_mutex) == -1 && errno == EINTR) {
            sem_post(sem_full);
            continue;
        }

        int idx = tbl->out;
        int item = tbl->buffer[idx];
        tbl->out = (tbl->out + 1) % CAPACITY;
        tbl->count--;

        printf("[consumer %d] consumed %d from index %d (count=%d)\n",
               getpid(), item, idx, tbl->count);
        fflush(stdout);

        consumed++;

        sem_post(sem_mutex);
        sem_post(sem_empty);

        struct timespec ts = {0, (long)(120 + rand() % 250) * 1000000L};
        nanosleep(&ts, NULL);
    }

    munmap(tbl, sizeof(shm_table_t));
    close(shm_fd);
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_close(sem_mutex);

    printf("[consumer %d] exiting (consumed=%ld)\n", getpid(), consumed);
    return 0;
}
