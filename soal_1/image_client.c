#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define PORT 8080
#define MAX_BUFFER 1024
#define DATABASE_DIR "database/"
#define LOG_FILE "server.log"

void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

void hex_to_ascii(char *hex, char *ascii) {
    int len = strlen(hex);
    for (int i = 0; i < len; i += 2) {
        sscanf(hex + i, "%2hhx", &ascii[i / 2]);
    }
    ascii[len / 2] = '\0';
}

void write_log(const char *source, const char *action, const char *info) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm_info);

    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
        fclose(log_file);
    }
}

void decrypt_and_save(char *hex_data, char *filename) {
    char reversed_hex[MAX_BUFFER];
    strcpy(reversed_hex, hex_data);
    reverse_string(reversed_hex);

    char ascii_data[MAX_BUFFER];
    hex_to_ascii(reversed_hex, ascii_data);

    FILE *jpeg_file = fopen(filename, "wb");
    if (jpeg_file) {
        fwrite(ascii_data, 1, strlen(ascii_data), jpeg_file);
        fclose(jpeg_file);
    }
}

int main() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    umask(0);
    setsid();

    mkdir(DATABASE_DIR, 0777);

    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    write_log("Server", "START", "Server started on port 8080");

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        char buffer[MAX_BUFFER] = {0};
        read(client_socket, buffer, MAX_BUFFER);

        if (strncmp(buffer, "DECRYPT:", 8) == 0) {
            char *hex_data = buffer + 8;
            char filename[50];
            sprintf(filename, "%s%ld.jpeg", DATABASE_DIR, time(NULL));
            decrypt_and_save(hex_data, filename);
            write_log("Server", "SAVE", filename);
            send(client_socket, filename, strlen(filename), 0);
        }
        else if (strncmp(buffer, "DOWNLOAD:", 9) == 0) {
            char *requested_file = buffer + 9;
            FILE *file = fopen(requested_file, "rb");
            if (file) {
                char file_data[MAX_BUFFER];
                size_t bytes_read = fread(file_data, 1, MAX_BUFFER, file);
                send(client_socket, file_data, bytes_read, 0);
                write_log("Server", "UPLOAD", requested_file);
                fclose(file);
            } else {
                send(client_socket, "ERROR: File not found", 21, 0);
                write_log("Server", "ERROR", "File not found");
            }
        }
        close(client_socket);
    }
    return 0;
}

 
