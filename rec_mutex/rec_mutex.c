#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rec_mutex.h"

int rec_mutex_init(rec_mutex_t *m) {
    m->t_lock = 0;
    m->id = 0;

    // posibles errores al inicializar los mutex y condiciones
    if (pthread_mutex_init(&(m->mutex), NULL) != 0) return -1;
    if (pthread_cond_init(&(m->cond), NULL) != 0) {
        pthread_mutex_destroy(&(m->mutex));  // destruir mutex si cond falla
        return -1;
    }

    return 0;
}

int rec_mutex_destroy(rec_mutex_t *m) {
    // posibles errores al destruir
    if (pthread_mutex_destroy(&(m->mutex)) != 0) return -1;
    if (pthread_cond_destroy(&(m->cond)) != 0) return -1;

    return 0;
}

int rec_mutex_lock(rec_mutex_t *m) {
    int err = pthread_mutex_lock(&(m->mutex));
    if (err != 0) return err;

    while (m->t_lock > 0 && !pthread_equal(m->id, pthread_self())) {
        err = pthread_cond_wait(&(m->cond), &(m->mutex));
        if (err != 0) {
            pthread_mutex_unlock(&(m->mutex));
            return err;  // Devolver error si wait falla
        }
    }

    m->t_lock++;
    m->id = pthread_self();

    pthread_mutex_unlock(&(m->mutex));
    return 0;
}

int rec_mutex_unlock(rec_mutex_t *m) {
    int err = pthread_mutex_lock(&(m->mutex));
    if (err != 0) return err;

    if (pthread_equal(m->id, pthread_self())) {
        m->t_lock--;
        if (m->t_lock <= 0) {
            m->id = 0;
            err = pthread_cond_broadcast(&(m->cond)); // Despertar a todos los hilos
            if (err != 0) {
                pthread_mutex_unlock(&(m->mutex));
                return err;
            }
        }
    }

    pthread_mutex_unlock(&(m->mutex));
    return 0;
}

int rec_mutex_trylock(rec_mutex_t *m) {
    int err = pthread_mutex_lock(&(m->mutex));
    if (err != 0) return err;

    if (m->t_lock != 0 && !pthread_equal(m->id, pthread_self())) {
        pthread_mutex_unlock(&(m->mutex));
        return EBUSY;  // Devuelve EBUSY si el mutex ya está bloqueado
    }

    m->id = pthread_self();
    m->t_lock++;

    pthread_mutex_unlock(&(m->mutex));
    return 0;
}
