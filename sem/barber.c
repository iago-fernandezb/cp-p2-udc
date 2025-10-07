#include "sem.h"
#include "options.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t customers;
sem_t barbero;
sem_t free_seats;
int cut_time;

void *barber_function(void *arg) {
    int id = *(int *)arg;
    while (1) {
        sem_p(&customers);  // Esperar clientes
        sem_p(&free_seats); // Tomar una silla

        printf("Barbero %d está cortando el pelo.\n", id);
        usleep(cut_time); // Simula el tiempo de corte de pelo

        sem_v(&barbero);   // Indica que el barbero está libre
        sem_v(&free_seats); // Libera la silla después del corte
    }
    return NULL;
}

void *customer_function(void *arg) {
    int id = *(int *)arg;
    if(sem_tryp(&free_seats) == 0){  // Intentar tomar una silla sin bloquear
        printf("Cliente %d tomó una silla.\n", id);
        sem_v(&customers);   // Notificar que hay un cliente esperando
        sem_p(&barbero);     // Esperar al barbero

        printf("Cliente %d se está cortando el pelo.\n", id);
        usleep(cut_time);  // Simula el corte de pelo

        sem_v(&free_seats); // Liberar la silla después del corte
    } else {
        printf("Cliente %d se fue, no hay sillas disponibles.\n", id);
    }

    return NULL;
}

int main(int argc, char **argv) {
    struct options opt;
    opt.barbers = 5;
    opt.customers = 10;
    opt.cut_time = 3000;

    read_options(argc, argv, &opt);
    
    cut_time = opt.cut_time;

    pthread_t barber_threads[opt.barbers];
    pthread_t customer_threads[opt.customers];
    int barber_ids[opt.barbers], customer_ids[opt.customers];

    sem_init(&customers, 0);
    sem_init(&barbero, 0);
    sem_init(&free_seats, opt.customers / 2);

    // Crear barberos con ID único
    for (int i = 0; i < opt.barbers; i++) {
        barber_ids[i] = i + 1;
        pthread_create(&barber_threads[i], NULL, barber_function, &barber_ids[i]);
    }

    // Crear clientes con ID único
    for (int i = 0; i < opt.customers; i++) {
        customer_ids[i] = i + 1;
        usleep(rand() % 1000);
        pthread_create(&customer_threads[i], NULL, customer_function, &customer_ids[i]);
    }

    // Esperar a que todos los clientes terminen
    for (int i = 0; i < opt.customers; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    exit(0);
}
