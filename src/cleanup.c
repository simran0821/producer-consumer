#include "common.h"

int main(void) {
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FULL_NAME);
    sem_unlink(SEM_MUTEX_NAME);
    shm_unlink(SHM_NAME);
    printf("[cleanup] unlinked shared memory and semaphores.\n");
    return 0;
}
