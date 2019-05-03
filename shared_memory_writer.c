#include <pthread.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <zconf.h>
#include <string.h>

void initFileIfNeed(const char* const desk) {
    FILE* file = fopen(desk, "w");
    assert(file != NULL);
    fclose(file);
}

void* initSharedMemory(int* shmid, size_t size, const char* const desk) {
    initFileIfNeed(desk);
    key_t key = ftok(desk,65);
    assert(key != -1);

    *shmid = shmget(key, size, 0755 | IPC_CREAT);
    assert(*shmid != -1);
    return shmat(*shmid, NULL, 0);
}

void initMtx(pthread_mutexattr_t* mta, pthread_mutex_t* mtx) {
    int rc = 0;

    rc = pthread_mutexattr_init(mta);
    assert(rc == 0);

    rc = pthread_mutexattr_setpshared(mta, PTHREAD_PROCESS_SHARED);
    assert(rc == 0);

    rc = pthread_mutex_init(mtx, mta);
    assert(rc == 0);
}

void initCondVar(pthread_condattr_t* condattr, pthread_cond_t* cond){
    int rc = 0;

    rc = pthread_condattr_init(condattr);
    assert(rc == 0);

    rc = pthread_condattr_setpshared(condattr, PTHREAD_PROCESS_SHARED);
    assert(rc == 0);

    rc = pthread_cond_init(cond, condattr);
    assert(rc == 0);
}

void destroyMutex(pthread_mutexattr_t* mta, pthread_mutex_t* mtx) {
    pthread_mutexattr_destroy(mta);
    pthread_mutex_destroy(mtx);
}

void destroyCondVar(pthread_condattr_t* condattr, pthread_cond_t* cond) {
    pthread_condattr_destroy(condattr);
    pthread_cond_destroy(cond);
}

void destroySharedMemory(int id, void* memory) {
    shmdt(memory);
    shmctl(id,IPC_RMID,NULL);
}

int main(int argc, char **argv)
{
    int mtxshmid = 0;
    int condvarshmid = 0;
    int datashmid = 0;

    pthread_mutexattr_t mta;
    pthread_condattr_t condattr;

    pthread_mutex_t* mtx = NULL;
    pthread_cond_t* cond = NULL;

    char* messageSegment = NULL;

    const int sharedDataSize = 1024;
    const char* const message = "Hello world";

    mtx = (pthread_mutex_t*)initSharedMemory(&mtxshmid, sizeof(pthread_mutex_t), "mtx");
    cond = (pthread_cond_t*)initSharedMemory(&condvarshmid, sizeof(pthread_cond_t), "cond");


    messageSegment = (char*)initSharedMemory(&datashmid, sharedDataSize, "data");

    initMtx(&mta, mtx);
    initCondVar(&condattr, cond);

    pthread_mutex_lock(mtx);
    printf("[Writer] Mutex obtained\n");
    sleep(5);
    pthread_mutex_unlock(mtx);
    printf("[Writer] Mutex unlocked\n");

    sleep(5);
    printf("[Writer] Writing data...\n");

    memcpy(messageSegment, message, strlen(message));
    printf("[Writer] Notifying...\n");
    pthread_cond_signal(cond);

    destroyMutex(&mta, mtx);
    destroySharedMemory(mtxshmid, mtx);

    destroyCondVar(&condattr, cond);
    destroySharedMemory(condvarshmid, cond);

    destroySharedMemory(datashmid, messageSegment);

    return 0;
}