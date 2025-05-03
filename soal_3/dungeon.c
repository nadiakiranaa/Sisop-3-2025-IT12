#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include "shop.c"

#define BUFFER_SIZE 5000

typedef struct {
    char name[32];
    int gold;
    int base_damage;
    int kills;
    int current_weapon_id;
    int inventory[10];
    int inventory_count;
} Player;

void show_stats(Player *p, char *out) {
    Weapon *w = get_weapon(p->current_weapon_id);
    if (!w) {
        sprintf(out, "\033[33mGold: %d\nBase Damage: %d\nKills: %d\nWeapon: \033[31mNone\033[0m\n\033[0m", p->gold, p->base_damage, p->kills);
    } else {
        sprintf(out, "\033[33mGold: %d\nBase Damage: %d\nKills: %d\nWeapon: \033[36m%s\033[0m\n", p->gold, p->base_damage, p->kills, w->name);
        if (w->has_passive) {
            strcat(out, "\033[35mPassive: ");
            strcat(out, w->passive);
            strcat(out, "\033[0m\n");
        }
    }
}

void show_inventory(Player *p, char *out) {
    strcpy(out, "\033[1;34m=== YOUR INVENTORY ===\033[0m\n");
    for (int i = 0; i < p->inventory_count; i++) {
        Weapon *w = get_weapon(p->inventory[i]);
        char temp[256];
        sprintf(temp, "\033[36m[%d]\033[0m %s (DMG: \033[32m%d\033[0m)", i + 1, w->name, w->damage);
        if (w->has_passive) {
            strcat(temp, " \033[35m(Passive: ");
            strcat(temp, w->passive);
            strcat(temp, ")\033[0m");
        }
        if (p->current_weapon_id == p->inventory[i]) {
            strcat(temp, " \033[1;33m(EQUIPPED)\033[0m");
        }
        strcat(temp, "\n");
        strcat(out, temp);
    } 
}
int buy_weapon(Player *p, int id) {
    Weapon *w = get_weapon(id);
    if (!w || p->gold < w->price) {
        printf("\033[31mPurchase failed. Insufficient gold or invalid weapon ID.\033[0m\n");
        return 0;

    }

    p->gold -= w->price;
    p->inventory[p->inventory_count++] = id;
    p->current_weapon_id = id;
    p->base_damage = w->damage;

    printf("\033[32mWeapon bought: %s\033[0m\n", w->name);
    printf("Updated stats:\nGold: %d\nBase Damage: %d\nCurrent Weapon: %s (ID: %d)\n",
           p->gold, p->base_damage, w->name, p->current_weapon_id);

    return 1;
}

void equip_weapon(Player *p, int id) {
    if (id < 0 || id >= p->inventory_count) return; // Cek apakah ID valid
    int weapon_id = p->inventory[id];
    Weapon *w = get_weapon(weapon_id);
    if (!w) return;

    p->current_weapon_id = weapon_id; // Set senjata yang dipilih sebagai senjata yang dipasang
    p->base_damage = w->damage; // Update base damage pemain sesuai dengan senjata yang dipilih

    printf("\033[32mWeapon equipped: %s\n\033[0m", w->name);
    printf("Updated stats: Gold: %d, Base Damage: %d, Current Weapon: %s\n",
           p->gold, p->base_damage, w->name);
}



void battle(Player *p, int sock) {
    int hp = 50 + rand() % 151;
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "\033[1;34m=== BATTLE MODE ===\033[0m\n\033[1;34mEnemy appears with HP: %d\033[0m\n", hp);
    send(sock, buffer, strlen(buffer), 0);
    while (1) {
        int bar_length = 20;
        int filled = (hp * bar_length) / 200;
        char bar[64] = "";
        for (int i = 0; i < bar_length; i++) {
            strcat(bar, i < filled ? "#" : "-");
        }

        sprintf(buffer, "\033[1;31mEnemy HP: [%s] %d\033[0m\n", bar, hp);
        send(sock, buffer, strlen(buffer), 0);

        bzero(buffer, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);

        if (strncmp(buffer, "exit", 4) == 0) break;
        if (strncmp(buffer, "attack", 6) != 0) {
            send(sock, "\033[33mInvalid command.\033[0m\n", 29, 0);
            continue;
        }

        int dmg = p->base_damage + rand() % 6;
        if (rand() % 5 == 0) {
            dmg *= 2;
            sprintf(buffer, "\033[1;31mCritical Hit! You dealt %d damage!\033[0m\n", dmg);
            send(sock, buffer, strlen(buffer), 0);
        } else {
            sprintf(buffer, "\033[1;32mYou dealt %d damage!\033[0m\n", dmg);
            send(sock, buffer, strlen(buffer), 0);
        }

        Weapon *w = get_weapon(p->current_weapon_id);
        if (w && w->has_passive) {
            sprintf(buffer, "\033[35mPassive activated: %s\033[0m\n", w->passive);
            send(sock, buffer, strlen(buffer), 0);
        }

        hp -= dmg;
        if (hp <= 0) {
            int gold = 50 + rand() % 51;
            p->gold += gold;
            p->kills++;
            sprintf(buffer, "\033[32mEnemy defeated! +%d gold\033[0m\n", gold);
            send(sock, buffer, strlen(buffer), 0);
void handle_client(int client_sock) {
    Player player = {"Hero", 500, 5, 0, -1, {0}, 0};
    char buffer[BUFFER_SIZE];

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (strncmp(buffer, "stats", 5) == 0) {
            show_stats(&player, buffer);
            send(client_sock, buffer, strlen(buffer), 0);

        } else if (strncmp(buffer, "shop", 4) == 0) {
            FILE *fp = fopen("shop.txt", "w+");
            show_shop(fp);
            rewind(fp);
            fread(buffer, 1, BUFFER_SIZE, fp);
            fclose(fp);
            send(client_sock, buffer, BUFFER_SIZE, 0);

        } else if (strncmp(buffer, "inv", 3) == 0) {
            show_inventory(&player, buffer);
            send(client_sock, buffer, strlen(buffer), 0);

            bzero(buffer, BUFFER_SIZE);
            recv(client_sock, buffer, BUFFER_SIZE, 0);

            if (strncmp(buffer, "0", 1) == 0) {
                // Cancel option
                send(client_sock, "\033[33mInventory menu canceled.\033[0m\n", 35, 0);
            } else {
                int weapon_idx = atoi(buffer) - 1;  // Convert to zero-based index
                if (weapon_idx >= 0 && weapon_idx < player.inventory_count) {
                    equip_weapon(&player, weapon_idx);
                    send(client_sock, "\033[32mWeapon equipped successfully!\033[0m\n", 39, 0);
                } else {
                    send(client_sock, "\033[31mInvalid selection!\033[0m\n", 30, 0);
                }
            }

        } else if (strncmp(buffer, "buy", 3) == 0) {
            char *second = strtok(buffer, " ");
            second = strtok(NULL, " ");
            int id = atoi(second);
            if (buy_weapon(&player, id - 1)) {
                snprintf(buffer, sizeof(buffer), "\033[32mWeapon bought!\033[0m\n");
                show_stats(&player, buffer + strlen(buffer));
                send(client_sock, buffer, strlen(buffer), 0);
            } else {
                send(client_sock, "\033[31mFailed to buy.\033[0m\n", 25, 0);
            }

        } else if (strncmp(buffer, "equip", 5) == 0) {
            int idx = atoi(buffer + 6);
            equip_weapon(&player, idx);
            send(client_sock, "\033[33mEquipped!\033[0m\n", 20, 0);

        } else if (strncmp(buffer, "battle", 6) == 0) {
            battle(&player, client_sock);

        } else {
            send(client_sock, "\033[31mInvalid command.\033[0m\n", 27, 0);
        }
    }
}
            hp = 50 + rand() % 151;
            snprintf(buffer, sizeof(buffer), "\033[1;34mA new enemy appears with HP: %d\033[0m\n", hp);
            send(sock, buffer, strlen(buffer), 0);
        }
    }
}

void handle_client(int client_sock) {
    Player player = {"Hero", 500, 5, 0, -1, {0}, 0};
    char buffer[BUFFER_SIZE];

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (strncmp(buffer, "stats", 5) == 0) {
            show_stats(&player, buffer);
            send(client_sock, buffer, strlen(buffer), 0);

        } else if (strncmp(buffer, "shop", 4) == 0) {
            FILE *fp = fopen("shop.txt", "w+");
            show_shop(fp);
            rewind(fp);
            fread(buffer, 1, BUFFER_SIZE, fp);
            fclose(fp);
            send(client_sock, buffer, BUFFER_SIZE, 0);

        } else if (strncmp(buffer, "inv", 3) == 0) {
            show_inventory(&player, buffer);
            send(client_sock, buffer, strlen(buffer), 0);

            bzero(buffer, BUFFER_SIZE);
            recv(client_sock, buffer, BUFFER_SIZE, 0);

            if (strncmp(buffer, "0", 1) == 0) {
                // Cancel option
                send(client_sock, "\033[33mInventory menu canceled.\033[0m\n", 35, 0);
            } else {
                int weapon_idx = atoi(buffer) - 1;  // Convert to zero-based index
                if (weapon_idx >= 0 && weapon_idx < player.inventory_count) {
                    equip_weapon(&player, weapon_idx);
                    send(client_sock, "\033[32mWeapon equipped successfully!\033[0m\n", 39, 0);
                } else {
                    send(client_sock, "\033[31mInvalid selection!\033[0m\n", 30, 0);
                }
            }

        } else if (strncmp(buffer, "buy", 3) == 0) {
            char *second = strtok(buffer, " ");
            second = strtok(NULL, " ");
            int id = atoi(second);
            if (buy_weapon(&player, id - 1)) {
                snprintf(buffer, sizeof(buffer), "\033[32mWeapon bought!\033[0m\n");
                show_stats(&player, buffer + strlen(buffer));
                send(client_sock, buffer, strlen(buffer), 0);
            } else {
                send(client_sock, "\033[31mFailed to buy.\033[0m\n", 25, 0);
            }

        } else if (strncmp(buffer, "equip", 5) == 0) {
            int idx = atoi(buffer + 6);
            equip_weapon(&player, idx);
            send(client_sock, "\033[33mEquipped!\033[0m\n", 20, 0);

        } else if (strncmp(buffer, "battle", 6) == 0) {
            battle(&player, client_sock);

        } else {
            send(client_sock, "\033[31mInvalid command.\033[0m\n", 27, 0);
        }
    }
}

void handle_inventory_input(Player *p, int sock) {
    char buffer[BUFFER_SIZE];
    int weapon_id = -1;

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        show_inventory(p, buffer);  // Tampilkan inventaris
        send(sock, buffer, strlen(buffer), 0);

        bzero(buffer, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);  // Terima input dari pemain

        if (strncmp(buffer, "0", 1) == 0) {
            send(sock, "\033[33mCancelled.\033[0m\n", 20, 0);  // Batalkan dan kembali ke menu utama
            break;
        } else {
            int weapon_choice = atoi(buffer) - 1;  // Kurangi 1 untuk sesuai dengan indeks array
            if (weapon_choice >= 0 && weapon_choice < p->inventory_count) {
                equip_weapon(p, weapon_choice);  // Equip senjata yang dipilih
                send(sock, "\033[32mWeapon equipped successfully!\033[0m\n", 39, 0);
                break;
            } else {
                send(sock, "\033[31mInvalid weapon choice.\033[0m\n", 30, 0);
            }
        }
    }
}
int main() {
    srand(time(NULL));
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(sockfd, 1);
    printf("\033[1;32mDungeon server running on port 8080...\033[0m\n");

    int client = accept(sockfd, NULL, NULL);
    printf("Client connected.\n");

    handle_client(client);

    close(client);
    close(sockfd);
    return 0;
}
