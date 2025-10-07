#ifndef __REC_MUTEX_H__
#define __REC_MUTEX_H__

#include <pthread.h>

typedef struct rec_mutex_t {
    int t_lock;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t id;
}rec_mutex_t;

int rec_mutex_init(rec_mutex_t *m);
int rec_mutex_destroy(rec_mutex_t *m);

int rec_mutex_lock(rec_mutex_t *m);
int rec_mutex_unlock(rec_mutex_t *m);
int rec_mutex_trylock(rec_mutex_t *m); // 0 if sucessful, -1 if already locked

#endif
