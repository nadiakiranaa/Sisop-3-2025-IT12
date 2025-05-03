// delivery_agent.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_ORDERS 100
#define MAX_NAME 100
#define MAX_ADDR 200
#define SHM_KEY 1234

typedef struct {
    char nama[MAX_NAME];
    char alamat[MAX_ADDR];
    char tipe[10];
    char status[100];
} Order;

typedef struct {
    Order orders[MAX_ORDERS];
    int total_orders;
} SharedData;

SharedData* data;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void log_delivery(const char* agent, const char* nama, const char* alamat) {
    FILE* log = fopen("delivery.log", "a");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(log, "[%02d/%02d/%d %02d:%02d:%02d] [%s] Express package delivered to %s in %s\n",
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            agent, nama, alamat);
    fclose(log);
}

void* agent_function(void* arg) {
    char* agent_name = (char*) arg;

    while (1) {
        pthread_mutex_lock(&lock);
        for (int i = 0; i < data->total_orders; i++) {
            if (strcmp(data->orders[i].tipe, "Express") == 0 &&
                strcmp(data->orders[i].status, "Pending") == 0) {

                char status[100];
                sprintf(status, "Delivered by %s", agent_name);
                strcpy(data->orders[i].status, status);
                log_delivery(agent_name, data->orders[i].nama, data->orders[i].alamat);
                printf("[%s] Express Order Delivered to %s (%s) ^^\n", agent_name, data->orders[i].nama, data->orders[i].alamat);
                sleep(1);
                break;
            }
        }
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shmid < 0) {
        perror("Shared memory tidak ditemukan. Jalankan dispatcher dulu ya syg^^.");
        exit(1);
    }

    data = (SharedData*) shmat(shmid, NULL, 0);

    pthread_t agents[3];
    char* names[] = {"AGENT A", "AGENT B", "AGENT C"};

    for (int i = 0; i < 3; i++) {
        pthread_create(&agents[i], NULL, agent_function, names[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(agents[i], NULL);
    }

    shmdt(data);
    return 0;
}
