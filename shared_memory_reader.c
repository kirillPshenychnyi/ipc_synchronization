#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <assert.h>
#include <zconf.h>
#include <pthread.h>

void * getSharedObject(const char* const desk, size_t size) {
    key_t key = ftok(desk, 65);
    assert(key != -1);

    int shmid = shmget(key, size, 0666|IPC_CREAT);
    assert(shmid != -1);

    return shmat(shmid, NULL, 0);
}

void disconnectSharedObject(void* mem) {
    shmdt(mem);
}

int main()
{
    const int sharedDataSize = 1024;

    pthread_mutex_t* mtx = (pthread_mutex_t *)getSharedObject("mtx", sizeof(pthread_mutex_t));
    pthread_cond_t* cond = (pthread_cond_t *)getSharedObject("cond", sizeof(pthread_cond_t));
    char* message = (char*)getSharedObject("data", sharedDataSize);

    printf("[Reader] Obtaining mutex...\n");
    pthread_mutex_lock(mtx);
    printf("[Reader] Waiting for a signal...\n");
    pthread_cond_wait(cond, mtx);
    printf("[Reader] Signal received! The data message: %s\n", message);
    pthread_mutex_unlock(mtx);

    disconnectSharedObject(mtx);
    disconnectSharedObject(cond);
    disconnectSharedObject(message);

    return 0;
}