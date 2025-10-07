#include <pthread.h>
#include "rw_mutex.h"

struct rw_mutex_t {
    pthread_mutex_t lock;         // Mutex para proteger la estructura
    pthread_cond_t readers_ok;    // Condición para los lectores
    pthread_cond_t writers_ok;    // Condición para los escritores
    int readers_count;            // Número de lectores activos
    int writer_active;            // Indicador de escritor activo
};

int rw_mutex_init(rw_mutex_t *m) {
    m->readers_count = 0;
    m->writer_active = 0;
    if (pthread_mutex_init(&m->lock, NULL) != 0) {
        return -1;
    }
    if (pthread_cond_init(&m->readers_ok, NULL) != 0) {
        pthread_mutex_destroy(&m->lock);
        return -1;
    }
    if (pthread_cond_init(&m->writers_ok, NULL) != 0) {
        pthread_mutex_destroy(&m->lock);
        pthread_cond_destroy(&m->readers_ok);
        return -1;
    }
    return 0;
}

int rw_mutex_destroy(rw_mutex_t *m) {
    if (pthread_mutex_destroy(&m->lock) != 0) {
        return -1;
    }
    if (pthread_cond_destroy(&m->readers_ok) != 0) {
        return -1;
    }
    if (pthread_cond_destroy(&m->writers_ok) != 0) {
        return -1;
    }
    return 0;
}

int rw_mutex_readlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);

    while (m->writer_active > 0) { // Espera hasta que no haya escritores
        pthread_cond_wait(&m->readers_ok, &m->lock);
    }

    m->readers_count++; // Un nuevo lector ha llegado

    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rw_mutex_writelock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);

    while (m->readers_count > 0 || m->writer_active) { // Espera hasta que no haya lectores ni escritores
        pthread_cond_wait(&m->writers_ok, &m->lock);
    }

    m->writer_active = 1; // Un escritor ha comenzado

    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rw_mutex_readunlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);

    m->readers_count--; // Un lector ha terminado

    if (m->readers_count == 0) {
        pthread_cond_signal(&m->writers_ok); // Despierta a un escritor si hay alguno esperando
    }

    pthread_mutex_unlock(&m->lock);
    return 0;
}

int rw_mutex_writeunlock(rw_mutex_t *m) {
    pthread_mutex_lock(&m->lock);

    m->writer_active = 0; // El escritor ha terminado
    pthread_cond_signal(&m->writers_ok); // Despierta a otros escritores si hay
    pthread_cond_broadcast(&m->readers_ok); // Despierta a los lectores

    pthread_mutex_unlock(&m->lock);
    return 0;
}
