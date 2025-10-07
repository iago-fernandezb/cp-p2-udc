#include "sem.h"
#include <pthread.h>
#include <stdlib.h>

int sem_init(sem_t *semaforo, int valor) {
    if (!semaforo) return -1;
    semaforo->valor = valor;
    pthread_mutex_init(&semaforo->mutex, NULL);
    pthread_cond_init(&semaforo->cond, NULL);
    return 0;
}

int sem_destroy(sem_t *semaforo) {
    if (!semaforo) return -1; //
    pthread_mutex_destroy(&semaforo->mutex);
    pthread_cond_destroy(&semaforo->cond);
    return 0;
}

int sem_p(sem_t *semaforo) { // Wait operation
    if (!semaforo) return -1;
    pthread_mutex_lock(&semaforo->mutex);
    while (semaforo->valor <= 0) {
        pthread_cond_wait(&semaforo->cond, &semaforo->mutex);
    }
    semaforo->valor--;
    pthread_mutex_unlock(&semaforo->mutex);
    return 0;
}

int sem_v(sem_t *semaforo) { // Signal operation
    if (!semaforo) return -1;
    pthread_mutex_lock(&semaforo->mutex);
    semaforo->valor++;
    pthread_cond_signal(&semaforo->cond);
    pthread_mutex_unlock(&semaforo->mutex);
    return 0;
}

int sem_tryp(sem_t *semaforo) {
    if (!semaforo) return -1;
    int result = -1;
    pthread_mutex_lock(&semaforo->mutex);
    if (semaforo->valor > 0) {
        semaforo->valor--;
        result = 0; // Éxito
    }
    pthread_mutex_unlock(&semaforo->mutex);
    return result;
}


