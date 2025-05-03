#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_WEAPONS 5

// Kode warna ANSI
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

typedef struct {
    char name[32];
    int damage;
    int price;
    int has_passive;
    char passive[128];
} Weapon;

Weapon weapons[NUM_WEAPONS] = {
    {"Sword", 10, 50, 1, "Increase Damage by 5%"},
    {"Axe", 12, 60, 1, "Critical Hit Chance +10%"},
    {"Bow", 8, 40, 1, "Double Attack Speed"},
    {"Flame Saber", 15, 100, 0, ""},
    {"Dagger", 6, 30, 0, ""}
};

// void init_shop() {
//     // Senjata sudah diinisialisasi di awal
// }

void show_shop(FILE *out) {
    fprintf(out, "\033[1;34m=== Available Weapons ===\033[0m\n");
    for (int i = 0; i < 5; i++) {
        fprintf(out, "\033[36m[%d]\033[0m \033[1m%s\033[0m (DMG: \033[32m%d\033[0m, Price: \033[33m%d\033[0m)",
                i + 1, weapons[i].name, weapons[i].damage, weapons[i].price);
        if (weapons[i].has_passive)
            fprintf(out, " \033[35m%s\033[0m", weapons[i].passive);
        fprintf(out, "\n");
    }
}

Weapon* get_weapon(int id) {
    if (id >= 0 && id < 5) {
        return &weapons[id];
    }
    return NULL;
}
