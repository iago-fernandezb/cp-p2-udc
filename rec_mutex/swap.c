#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "op_count.h"
#include "options.h"
#include "rec_mutex.h"

double tf = 0;

double microsegundos() {
    struct timeval t;
    if (gettimeofday(&t, NULL) < 0)
        return 0.0;
    return (t.tv_usec + t.tv_sec * 1000000.0);
}

struct buffer {
    int *data;
    rec_mutex_t *mutex;
    int size;
};

struct thread_info {
    pthread_t thread_id;
    int thread_num;
};

struct args {
    int thread_num;
    int delay;
    int iterations;
    struct buffer *buffer;
};

void *swap(void *ptr) {
    struct args *args = ptr;
    while (args->iterations--) {
        int i, j, tmp;
        do {
            i = rand() % args->buffer->size;
            j = rand() % args->buffer->size;
        } while (i == j);

        if (i > j) {
            int temp = i;
            i = j;
            j = temp;
        }

        rec_mutex_lock(&args->buffer->mutex[i]);
        rec_mutex_lock(&args->buffer->mutex[j]);

        printf("Thread %d swapping positions %d (== %d) and %d (== %d)\n",
               args->thread_num, i, args->buffer->data[i], j, args->buffer->data[j]);

        tmp = args->buffer->data[i];
        if (args->delay) usleep(args->delay);

        args->buffer->data[i] = args->buffer->data[j];
        if (args->delay) usleep(args->delay);

        args->buffer->data[j] = tmp;
        if (args->delay) usleep(args->delay);

        rec_mutex_unlock(&args->buffer->mutex[i]);
        rec_mutex_unlock(&args->buffer->mutex[j]);

        inc_count();
    }
    return NULL;
}

int cmp(int *e1, int *e2) {
    if (*e1 == *e2) return 0;
    return (*e1 < *e2) ? -1 : 1;
}

void print_buffer(struct buffer buffer) {
    for (int i = 0; i < buffer.size; i++)
        printf("%i ", buffer.data[i]);
    printf("\n");
}

void start_threads(struct options opt) {
    int i;
    struct thread_info *threads;
    struct args *args;
    struct buffer buffer;

    srand(time(NULL));

    if ((buffer.data = malloc(opt.buffer_size * sizeof(int))) == NULL) {
        printf("Out of memory\n");
        exit(1);
    }
    buffer.size = opt.buffer_size;
    rec_mutex_t mutex[buffer.size];

    for (i = 0; i < buffer.size; i++) {
        buffer.data[i] = i;
        rec_mutex_init(&mutex[i]);
    }
    buffer.mutex = mutex;

    printf("Creating %d threads\n", opt.num_threads);
    threads = malloc(sizeof(struct thread_info) * opt.num_threads);
    args = malloc(sizeof(struct args) * opt.num_threads);

    if (threads == NULL || args == NULL) {
        printf("Not enough memory\n");
        exit(1);
    }

    printf("Buffer before: ");
    print_buffer(buffer);
    double t1 = microsegundos();

    for (i = 0; i < opt.num_threads; i++) {
        threads[i].thread_num = i;

        args[i].thread_num = i;
        args[i].buffer = &buffer;
        args[i].delay = opt.delay;
        args[i].iterations = opt.iterations;

        if (pthread_create(&threads[i].thread_id, NULL, swap, &args[i])) {
            printf("Could not create thread #%d", i);
            exit(1);
        }
    }

    for (i = 0; i < opt.num_threads; i++) {
        pthread_join(threads[i].thread_id, NULL);
    }

    double t2 = microsegundos();
    tf = t2 - t1;

    printf("Buffer after:  ");
    qsort(buffer.data, opt.buffer_size, sizeof(int), (int (*)(const void *, const void *))cmp);
    print_buffer(buffer);

    printf("Iterations: %d\n", get_count());
    printf("Elapsed time: %.0f microseconds\n", tf);

    free(args);
    free(threads);
    free(buffer.data);

    for (i = 0; i < buffer.size; i++) {
        rec_mutex_destroy(&mutex[i]);
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    struct options opt;

    opt.num_threads = 10;
    opt.buffer_size = 10;
    opt.iterations = 100;
    opt.delay = 10;

    read_options(argc, argv, &opt);
    start_threads(opt);

    exit(0);
}
