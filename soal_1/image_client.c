#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER 1024

int main() {
    int client_fd;
    struct sockaddr_in address;
    char buffer[MAX_BUFFER] = {0};

    while (1) {
        printf("\n[ Image Decoder Client ]\n");
        printf("1. Send input file to server\n");
        printf("2. Download file from server\n");
        printf("3. Exit\n>> ");

        int choice;
        scanf("%d", &choice);
        getchar();

        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

        if (connect(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            printf("Gagal connect ke server!\n");
            continue;
        }

        switch (choice) {
            case 1: {
                printf("Enter the file name (e.g., input_1.txt): ");
                char input_filename[100];
                scanf("%99s", input_filename);

                char filename[150];
                sprintf(filename, "/home/hansen/SisopModul3/client/secrets/%s", input_filename);
                FILE *file = fopen(filename, "rb");
                if (!file) {
                    printf("Error: File tidak ditemukan di client/secrets/!\n");
                    close(client_fd);
                    break;
                }

                char hex_data[MAX_BUFFER];
                fgets(hex_data, MAX_BUFFER, file);
                fclose(file);

                sprintf(buffer, "DECRYPT:%s", hex_data);
                send(client_fd, buffer, strlen(buffer), 0);

                char response[MAX_BUFFER] = {0};
                read(client_fd, response, MAX_BUFFER);
                printf("Server: File disimpan sebagai %s\n", response);
                break;
            }
            case 2: {
                printf("Enter the file name (e.g., 1744493652.jpeg): ");
                char output_filename[100];
                scanf("%99s", output_filename);

                sprintf(buffer, "DOWNLOAD:%s", output_filename);
                send(client_fd, buffer, strlen(buffer), 0);

                char jpeg_data[MAX_BUFFER] = {0};
                read(client_fd, jpeg_data, MAX_BUFFER);

                if (strncmp(jpeg_data, "ERROR:", 6) == 0) {
                    printf("%s\n", jpeg_data);
                } else {
                    char filename[150];
                    sprintf(filename, "client/%s", output_filename);
                    FILE *output = fopen(filename, "wb");
                    fwrite(jpeg_data, 1, strlen(jpeg_data), output);
                    fclose(output);
                    printf("Success! File disimpan sebagai %s\n", filename);
                }
                break;
            }
            case 3:
                close(client_fd);
                exit(0);
            default:
                printf("Pilihan tidak valid!\n");
        }
        close(client_fd);
    }
    return 0;
}
