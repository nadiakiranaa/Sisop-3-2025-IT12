#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFFER_SIZE 5000

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));

    char cmd[128], buffer[BUFFER_SIZE];

    while (1) {
        printf("\n--- The Lost Dungeon ---\n");
        printf("1. Show Player Stats\n");
        printf("2. Shop (Buy Weapon)\n");
        printf("3. View Inventory & Equip Weapon\n");
        printf("4. Battle Mode\n");
        printf("5. Exit\n");
        printf("Select: ");
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\n")] = 0;

        if (strcmp(cmd, "1") == 0) {
            strcpy(cmd, "stats");

        } else if (strcmp(cmd, "2") == 0) {
            strcpy(cmd, "shop");
            send(sockfd, cmd, strlen(cmd), 0);

            bzero(buffer, BUFFER_SIZE);
            recv(sockfd, buffer, BUFFER_SIZE, 0);
            printf("%s", buffer);

            printf("Select Weapon ID to buy (1-5): ");
            fgets(cmd, sizeof(cmd), stdin);
            int id = atoi(cmd);

            if (id >= 1 && id <= 5) {
                sprintf(cmd, "buy %d", id);
                send(sockfd, cmd, strlen(cmd), 0);
                bzero(buffer, BUFFER_SIZE);
                recv(sockfd, buffer, BUFFER_SIZE, 0);
                printf("%s", buffer);
            } else {
                printf("Invalid Weapon ID.\n");
            }

            continue;

        } else if (strcmp(cmd, "3") == 0) {
            strcpy(cmd, "inv");
            send(sockfd, cmd, strlen(cmd), 0);

            // Terima daftar inventory
            bzero(buffer, BUFFER_SIZE);
            recv(sockfd, buffer, BUFFER_SIZE, 0);
            printf("%s", buffer);

            // Input pilihan senjata untuk equip
            printf("Select weapon number to equip (0 to cancel): ");
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strcspn(cmd, "\n")] = 0;

            // Kirim pilihan ke server
            send(sockfd, cmd, strlen(cmd), 0);

            // Terima respon konfirmasi equip
            bzero(buffer, BUFFER_SIZE);
            recv(sockfd, buffer, BUFFER_SIZE, 0);
            printf("%s", buffer);

            continue;

        } else if (strcmp(cmd, "4") == 0) {
            strcpy(cmd, "battle");
            send(sockfd, cmd, strlen(cmd), 0);

            while (1) {
                bzero(buffer, BUFFER_SIZE);
                recv(sockfd, buffer, BUFFER_SIZE, 0);
                printf("%s", buffer);

                printf("Command [attack/exit]: > ");
                fgets(cmd, sizeof(cmd), stdin);
                cmd[strcspn(cmd, "\n")] = 0;

                send(sockfd, cmd, strlen(cmd), 0);

                if (strcmp(cmd, "exit") == 0) break;
            }

            continue;

        } else if (strcmp(cmd, "5") == 0) {
            break;

         } else {
            printf("\033[31mInvalid option. Please, try again!\033[0m\n");
            continue;
        }

        send(sockfd, cmd, strlen(cmd), 0);
        bzero(buffer, BUFFER_SIZE);
        recv(sockfd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
    }

    close(sockfd);
    return 0;
}
