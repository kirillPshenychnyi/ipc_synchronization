
#include <stdlib.h>
#include <pthread.h>
#include <zconf.h>

void* writerProcess(void* args) {
    system("./shared_memory_writer");
}

void* readerProcess(void* args) {
    system("./shared_memory_reader");
}

int main() {
    pthread_t reader, writer;

    pthread_create(&writer, NULL, writerProcess, NULL);

    // Sleep 1 second so writer process starts first
    sleep(1);
    pthread_create(&reader, NULL, readerProcess, NULL);

    pthread_join(reader, NULL);
    pthread_join(writer, NULL);

    return 0;
}