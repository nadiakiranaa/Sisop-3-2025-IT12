// dispatcher.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define SHM_KEY 1234

void download_csv_if_needed() {
    FILE* f = fopen("delivery_order.csv", "r");
    if (f) {
        fclose(f);
        return;
    }

    printf("Aduh! File CSV tidak ditemukan. dwnld file .csv instead...\n");
    int result = system("wget -O delivery_order.csv \"https://drive.usercontent.google.com/u/0/uc?id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9&export=download\"");
    if (result != 0) {
        fprintf(stderr, "Gagal mengunduh file CSV.\n");
        exit(1);
    }
}

typedef struct {
    char nama[100];
    char alamat[200];
    char tipe[10];
    char status[100];
} Order;

typedef struct {
    Order orders[100];
    int total_orders;
} SharedData;

void log_delivery(const char* agent, const char* nama, const char* alamat, const char* tipe) {
    FILE* log = fopen("delivery.log", "a");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(log, "[%02d/%02d/%d %02d:%02d:%02d] [%s] %s package delivered to %s in %s\n",
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            agent, tipe, nama, alamat);
    fclose(log);
}

void load_csv_to_shared_memory(SharedData* data) {
    FILE* file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Yachh file .CSV tidak ditemukan ;(");
        exit(1);
    }

    char line[512];
    int index = 0;

    while (fgets(line, sizeof(line), file)) {
        if (index >= 100) break;

        char* nama = strtok(line, ",");
        char* alamat = strtok(NULL, ",");
        char* tipe = strtok(NULL, "\n");

        strcpy(data->orders[index].nama, nama);
        strcpy(data->orders[index].alamat, alamat);
        strcpy(data->orders[index].tipe, tipe);
        strcpy(data->orders[index].status, "Pending");
        index++;
    }

    data->total_orders = index;
    fclose(file);
}

int main(int argc, char* argv[]) {
	download_csv_if_needed();
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    SharedData* data = (SharedData*) shmat(shmid, NULL, 0);

    if (data->total_orders == 0) {
        load_csv_to_shared_memory(data);
        printf("Yeay! CSV berhasil dimuat ke shared memory.\n");
    }

    if (argc < 2) {
        printf("Gunakan: \n./dispatcher -deliver [Nama], -status [Nama], -list\n");
        shmdt(data);
        return 0;
    }

    if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
        char* nama_user = argv[2];
        for (int i = 0; i < data->total_orders; i++) {
            if (strcmp(data->orders[i].nama, nama_user) == 0 && strcmp(data->orders[i].tipe, "Reguler") == 0) {
                if (strcmp(data->orders[i].status, "Pending") == 0) {
                    char status[100];
                    sprintf(status, "Delivered by Agent %s", getenv("USER"));
                    strcpy(data->orders[i].status, status);

                    log_delivery(getenv("USER"), data->orders[i].nama, data->orders[i].alamat, "Reguler");
                    printf("[%s] Reguler Order Delivered to %s (%s) ^^\n", getenv("USER"), nama_user, data->orders[i].alamat);
                    //printf("Pesanan %s berhasil dikirim oleh Agent %s.\n", nama_user, getenv("USER"));
                } else {
                    printf("Pesanan sudah dikirim.\n");
                }
                shmdt(data);
                return 0;
            }
        }
        printf("Pesanan tidak ditemukan.\n");
    } else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        char* nama = argv[2];
        for (int i = 0; i < data->total_orders; i++) {
            if (strcmp(data->orders[i].nama, nama) == 0) {
                printf("Status for %s: %s ^_^\n", nama, data->orders[i].status);
                shmdt(data);
                return 0;
            }
        }
        printf("Pesanan tidak ditemukan.\n");
    } else if (strcmp(argv[1], "-list") == 0) {
        printf("Daftar Semua Pesanan:\n");
        printf("---------------------------------------------------------------------\n");
        printf("| %-20s | %-15s | %-10s | %-9s |\n", "Nama", "Alamat", "Tipe", "Status");
        printf("---------------------------------------------------------------------\n");
        for (int i = 0; i < data->total_orders; i++) {
            //printf("%s - %s\n", data->orders[i].nama, data->orders[i].status);

            printf("| %-20s | %-15s | %-10s | %-9s |\n",
                data->orders[i].nama,
                data->orders[i].alamat,
                data->orders[i].tipe,
                data->orders[i].status);
        
        }
	printf("---------------------------------------------------------------------\n");
    } else {
        printf("Argumen tidak dikenali.\nGunakan:\n./dispatcher -deliver [nama]\n./dispatcher -status [nama]\n./dispatcher -list");
    }

    shmdt(data);
    return 0;
}
