#include "common.h"

static volatile sig_atomic_t keep_running = 1;
static void on_sigint(int sig) { (void)sig; keep_running = 0; }

int main(int argc, char *argv[]) {
    signal(SIGINT, on_sigint);

    long target = 0;
    if (argc >= 2) {
        target = strtol(argv[1], NULL, 10);
        if (target < 0) target = 0;
    }

    int created = 0;
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (shm_fd == -1 && errno == EEXIST) {
        shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("producer: shm_open existing");
            return 1;
        }
    } else if (shm_fd >= 0) {
        created = 1;
        if (ftruncate(shm_fd, (off_t)sizeof(shm_table_t)) == -1) {
            perror("producer: ftruncate");
            return 1;
        }
    } else {
        perror("producer: shm_open");
        return 1;
    }

    shm_table_t *tbl = mmap(NULL, sizeof(shm_table_t),
                            PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (tbl == MAP_FAILED) {
        perror("producer: mmap");
        return 1;
    }

    int sem_created = 0;
    sem_t *sem_empty = sem_open(SEM_EMPTY_NAME, O_CREAT | O_EXCL, 0666, CAPACITY);
    if (sem_empty == SEM_FAILED && errno == EEXIST) {
        sem_empty = sem_open(SEM_EMPTY_NAME, 0);
        if (sem_empty == SEM_FAILED) { perror("producer: sem_open empty"); return 1; }
    } else if (sem_empty != SEM_FAILED) {
        sem_created = 1;
    } else {
        perror("producer: sem_open empty create");
        return 1;
    }

    sem_t *sem_full = sem_open(SEM_FULL_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (sem_full == SEM_FAILED && errno == EEXIST) {
        sem_full = sem_open(SEM_FULL_NAME, 0);
        if (sem_full == SEM_FAILED) { perror("producer: sem_open full"); return 1; }
    } else if (sem_full == SEM_FAILED) {
        perror("producer: sem_open full create");
        return 1;
    }

    sem_t *sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (sem_mutex == SEM_FAILED && errno == EEXIST) {
        sem_mutex = sem_open(SEM_MUTEX_NAME, 0);
        if (sem_mutex == SEM_FAILED) { perror("producer: sem_open mutex"); return 1; }
    } else if (sem_mutex == SEM_FAILED) {
        perror("producer: sem_open mutex create");
        return 1;
    }

    if (created) {
        tbl->in = 0;
        tbl->out = 0;
        tbl->count = 0;
    }

    printf("[producer %d] started (target=%ld, capacity=%d)\n", getpid(), target, CAPACITY);
    fflush(stdout);

    long produced = 0;
    int next_item = 1;
    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    while (keep_running && (target == 0 || produced < target)) {
        if (sem_wait(sem_empty) == -1 && errno == EINTR) continue;
        if (sem_wait(sem_mutex) == -1 && errno == EINTR) {
            sem_post(sem_empty);
            continue;
        }

        int idx = tbl->in;
        tbl->buffer[idx] = next_item;
        tbl->in = (tbl->in + 1) % CAPACITY;
        tbl->count++;

        printf("[producer %d] produced %d at index %d (count=%d)\n",
               getpid(), next_item, idx, tbl->count);
        fflush(stdout);

        next_item++;
        produced++;

        sem_post(sem_mutex);
        sem_post(sem_full);

        struct timespec ts = {0, (long)(100 + rand() % 200) * 1000000L};
        nanosleep(&ts, NULL);
    }

    munmap(tbl, sizeof(shm_table_t));
    close(shm_fd);
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_close(sem_mutex);

    printf("[producer %d] exiting (produced=%ld)\n", getpid(), produced);
    return 0;
}
