# Sisop-3-2025-IT12

### Soal Modul 3 Sistem Operasi 2025

## Anggota
| Nama                            | NRP        |
|---------------------------------|------------|
| Nadia Kirana Afifah Prahandita  | 5027241005 |
| Hansen Chang                    | 5027241028 |
| Muhammad Khosyi Syehab          | 5027241089 |

## PENJELASAN

## Soal_1
## Soal_2
A. Mengunduh File Order dan Menyimpannya ke Shared Memory.

pertama kita akan cek keberadaan dari ``file .csv`` apakah sudah ada, jika belum maka akan download terlebih dahulu menggunakan ``wget``.
```
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
```
kemudian simpan di shared memory menggunakan, 
```
int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
SharedData* data = (SharedData*) shmat(shmid, NULL, 0);
```
Menggunakan shmget dengan IPC_CREAT untuk membuat segmen shared memory baru jika belum ada. shmat mengaitkan shared memory ke proses. lalu muat data csv ke shared memory.
```
void load_csv_to_shared_memory(SharedData* data) {
    ...
    while (fgets(line, sizeof(line), file)) {
        ...
        strcpy(data->orders[index].nama, nama);
        strcpy(data->orders[index].alamat, alamat);
        strcpy(data->orders[index].tipe, tipe);
        strcpy(data->orders[index].status, "Pending");
        ...
    }
    data->total_orders = index;
}
```
yang menyimpan semua entri ke ``data->orders``

B. Pengiriman Bertipe Express menggunakan ``delivery_agent.c``

Menyimpan semua pesanan dalam shared memory dimana order mencakup nama, alamat, tipe, dan status.
```
typedef struct {
    Order orders[MAX_ORDERS];
    int total_orders;
} SharedData;

```
kemudian kita membuat tiga thread sesuai agen yang dibutuhkan yakni, Agen A, Agen B, dan Agen C yang akan tercatat di log
```
pthread_t agents[3];
char* names[] = {"AGENT A", "AGENT B", "AGENT C"};

for (int i = 0; i < 3; i++) {
    pthread_create(&agents[i], NULL, agent_function, names[i]);
}
void* agent_function(void* arg) {
    ...
    if (strcmp(data->orders[i].tipe, "Express") == 0 &&
        strcmp(data->orders[i].status, "Pending") == 0) {
        ...
        strcpy(data->orders[i].status, status);
        log_delivery(agent_name, data->orders[i].nama, data->orders[i].alamat);
    }
    ...
}
fprintf(log, "[%02d/%02d/%d %02d:%02d:%02d] [%s] Express package delivered to %s in %s\n", ...)
```

C. Pengiriman Reguler menggunakan ``dispatcher.c``

Mengambil nama agen dari environment variable USER dan menulis log ke delivery.log. dengan format yang sesuai dengan soal.
```
if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
    char* nama_user = argv[2];
for (int i = 0; i < data->total_orders; i++) {
    if (strcmp(data->orders[i].nama, nama_user) == 0 && 
        strcmp(data->orders[i].tipe, "Reguler") == 0) {
sprintf(status, "Delivered by Agent %s", getenv("USER"));
strcpy(data->orders[i].status, status);
log_delivery(getenv("USER"), data->orders[i].nama, data->orders[i].alamat, "Reguler");
```

D. Mengecek status pemesanan
```
if (strcmp(argv[1], "-status") == 0 && argc == 3) {
    ...
    printf("Status for %s: %s ^_^\n", nama, data->orders[i].status);
```
Mengecek nama dan menampilkan status terkini dari pesanan.

E. Melihat semua daftar pesanan

Menampilkan semua pesanan lengkap dengan status, tipe, alamat, dan nama ke dalam shared memory.
```
if (strcmp(argv[1], "-list") == 0) {
    printf("Daftar Semua Pesanan:\n");
    ...
    for (int i = 0; i < data->total_orders; i++) {
        printf("| %-20s | %-15s | %-10s | %-9s |\n",
            data->orders[i].nama,
            data->orders[i].alamat,
            data->orders[i].tipe,
            data->orders[i].status);
    }
```
semua barang yang sudah diantar akan berubah statusnya di dalam shared memory sesuai dengan agen yang mengantar.
## Soal_3
A. Koneksi Client-Server (RPC Dungeon)
Player.c (client) harus terkoneksi ke dungeon.c (server) via socket TCP dan Server harus menangani beberapa client.
```
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
...
connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
```
B. Main Menu Interaktif
Client menampilkan menu utama dengan pilihan seperti: Stats, Shop, Inventory, Battle, Exit.
```
printf("\n--- The Lost Dungeon ---\n");
printf("1. Show Player Stats\n");
printf("2. Shop (Buy Weapon)\n");
...
```
C. Show Player Stats
Menampilkan informasi pemain: uang, senjata yang sedang dipakai, base damage, jumlah musuh dikalahkan.
```
if (strcmp(cmd, "1") == 0) {
    strcpy(cmd, "stats");
}
...
send(sockfd, cmd, strlen(cmd), 0);
recv(sockfd, buffer, BUFFER_SIZE, 0);
printf("%s\n", buffer);
```
D. Weapon Shop
Toko menyediakan 5+ senjata, 2+ senjata harus memiliki passive dan informasi yang ditampilkan: nama, damage, harga, dan passive (jika ada).
Kode  shop.c
```
Weapon weapons[NUM_WEAPONS] = {
    {"Sword", 10, 50, 1, "Increase Damage by 5%"},
    {"Axe", 12, 60, 1, "Critical Hit Chance +10%"},
    ...
};

void show_shop(FILE *out) {
    for (int i = 0; i < 5; i++) {
        ...
        if (weapons[i].has_passive)
            fprintf(out, " \033[35m%s\033[0m", weapons[i].passive);
    }
}
```
Kode Player.c
```
strcpy(cmd, "shop");
send(sockfd, cmd, strlen(cmd), 0);
recv(sockfd, buffer, BUFFER_SIZE, 0);
printf("%s", buffer);
```
E. Inventory & Equip Weapon
Inventory menampilkan senjata yang dimiliki dan efek passive & pemain bisa memilih senjata untuk dipakai.
```
strcpy(cmd, "inv");
send(sockfd, cmd, strlen(cmd), 0);
recv(sockfd, buffer, BUFFER_SIZE, 0);
printf("%s", buffer);

printf("Select weapon number to equip (0 to cancel): ");
...
send(sockfd, cmd, strlen(cmd), 0);
recv(sockfd, buffer, BUFFER_SIZE, 0);
printf("%s", buffer);
```
F.  Battle Mode
Pemain bisa menyerang musuh yang HP-nya acak (misal 50–200) dan ada sistem critical, damage random, reward random.
```
strcpy(cmd, "battle");
send(sockfd, cmd, strlen(cmd), 0);

while (1) {
    recv(sockfd, buffer, BUFFER_SIZE, 0);
    printf("%s", buffer);

    printf("Command [attack/exit]: > ");
    ...
    send(sockfd, cmd, strlen(cmd), 0);
    if (strcmp(cmd, "exit") == 0) break;
}
```
G.  Damage Equation + Critical + Passive
Damage berbasis base damage + random, Critical (misal 20%) melipatgandakan damage, Passive aktif berdasarkan efek senjata.
```
int base_dmg = player.base_dmg;
int dmg = base_dmg + rand() % 5;
if (weapon.has_crit && rand() % 100 < 20) dmg *= 2;
```
H.  Reward dan Health Musuh 
HP musuh acak, reward juga acak (gold) dan Setelah menang, musuh baru muncul.
```
int enemy_hp = 50 + rand() % 151; // 50 - 200
int gold_reward = 10 + rand() % 41; // 10 - 50
```
I.Error Handling
Tangani input yang tidak valid, seperti menu salah, weapon ID salah, command battle salah.
```
if (id < 1 || id > 5) {
    printf("Invalid Weapon ID.\n");
}
...
else {
    printf("\033[31mInvalid option. Please, try again!\033[0m\n");
}
```
## Soal_4
