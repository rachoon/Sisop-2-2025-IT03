<table>
  <thead>
    <tr>
      <th>No</th>
      <th>Nama</th>
      <th>NRP</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>1</td>
      <td>Zein muhammad hasan</td>
      <td>5027241035</td>
    </tr>
    <tr>
      <td>2</td>
      <td>Afriza Tristan Calendra Rajasa</td>
      <td>5027241104</td>
    </tr>
    <tr>
      <td>3</td>
      <td>Jofanka Al-kautsar Pangestu Abady</td>
      <td>5027241107</td>
    </tr>
  </tbody>
</table>

<nav>
  <ul>
    <li><a href="#soal1">Soal1</a></li>
    <li><a href="#soal2">Soal2</a></li>
    <li><a href="#soal3">Soal3</a></li>
    <li><a href="#soal4">Soal4</a></li>
  </ul>
</nav>


  <h2 id="soal1">Soal1</h2>

  <h2 id="soal2">Soal2</h2>

<p>
  Pada soal ini kita diminta untuk membantu perempuan yang nolep yang bernama Kanade Yoisaki. Kita diminta untuk membantu Kanade Yoisaki dan teman temannya untuk membuat prgram dengan bahasa c dengan berbeagai fitur, yaitu: <br>
  a. Download dan unzip sebuah starterkit. <br>
  b. Membuat directory karantina yang dapat mendecrypt nama file menggunakan algoritma base64. <br>
  c. Memindahkan file yang ada pada directory starter kit ke karantina dan sebaliknya. <br>
  d. Menghapus seluruh file yang ada pada directory karantina. <br>
  e. Mematikan program decrypt secara aman berdasarkan PID dri proses tersebut. <br>
  f. Membuat error handling. <br>
  g. mencatat penggunaan program dan log dari setiap penggunaan, lalu menyimpannya ke dalam file bernama activity.log.

  </p>
  <h3>Download dan unzip sebuah starterkit</h3>
```c
  void download_and_unzip() {
    printf("Mengunduh file dari Google Drive...\n");

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // proses untuk wget
        char *wget_args[] = {
            "wget", "-O", "starterkit.zip",
            "https://drive.usercontent.google.com/download?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&confirm=t",
            NULL
        };
        execvp(wget_args[0], wget_args);
        perror("execvp wget gagal");
        exit(EXIT_FAILURE);
    } else if (pid1 < 0) {
        perror("fork wget gagal");
        return;
    } else {
        // tunggu wget selesai
        waitpid(pid1, NULL, 0);
    }

    // ngecek dan buat folder starter_kit jika belum ada
    struct stat st = {0};
    if (stat("starter_kit", &st) == -1) {
        mkdir("starter_kit", 0700);
    }

    printf("Mengekstrak starterkit.zip ke folder starter_kit...\n");

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // proses untuk unzip file
        char *unzip_args[] = {
            "unzip", "-q", "starterkit.zip", "-d", "starter_kit",
            NULL
        };
        execvp(unzip_args[0], unzip_args);
        perror("execvp unzip gagal");
        exit(EXIT_FAILURE);
    } else if (pid2 < 0) {
        perror("fork unzip gagal");
        return;
    } else {
        // menunggu unzip selesai
        waitpid(pid2, NULL, 0);
    }

    // menghapus file dari sistem
    remove("starterkit.zip");
    printf("Selesai!\n");
}
```

  <h3>Membuat directory karantina yang dapat mendecrypt nama file menggunakan algoritma base64</h3>

  <h3>Memindahkan file yang ada pada directory starter kit ke karantina dan sebaliknya</h3>

  <h3>Menghapus seluruh file yang ada pada directory karantina</h3>

  <h3>Mematikan program decrypt secara aman berdasarkan PID dri proses tersebut</h3>

  <h3>Membuat error handling</h3>

  <h3>mencatat penggunaan program dan log dari setiap penggunaan, lalu menyimpannya ke dalam file bernama activity.log</h3>

```c

```
  
  <h2 id="soal3">Soal3</h2>

  <h2 id="soal4">Soal4</h2>

<p>
  Pada soal ini kita diminta untuk membuat sebuah program dalam bahasa c yang memiliki beberapa fitur yaitu: <br>
  A. list program yang sedang berjalan pada komputer user. <br>
  B. Daemon untuk meencatat log program yang sedang berjalan. <br>
  C. Fitur untuk mematikan daemon sebelumnya. <br>
  D. Program untuk menggagalkan semua proses yang sedang berjalan dan menulis status proses ke dalam file log dengan status FAILED. <br>
  E. Fitur untuk mengembalikan komputer seperti awal. <br>
  F. Mencatat semua aktivitas di file log.
</p>

<h3>4A.</h3>

```c
void list_processes(const char *username) {
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("opendir /proc");
        exit(EXIT_FAILURE);
    }

    uid_t uid = get_uid(username);
    struct dirent *entry;
    long ticks_per_sec = sysconf(_SC_CLK_TCK);

    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;

        char path[256], name[100] = "";
        uid_t proc_uid = -1;
        unsigned long vsize = 0; // for RAM
        long utime = 0, stime = 0, total_time = 0; // for CPU

        // --- Ambil UID dan Nama Proses ---
        snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
        FILE *fp = fopen(path, "r");
        if (!fp) continue;

        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "Name:", 5) == 0)
                sscanf(line, "Name:\t%99s", name);
            if (strncmp(line, "Uid:", 4) == 0)
                sscanf(line, "Uid:\t%d", &proc_uid);
            if (strncmp(line, "VmRSS:", 6) == 0)
                sscanf(line, "VmRSS:\t%lu", &vsize); // dalam KB
        }
        fclose(fp);

        if (proc_uid != uid) continue;

        // --- Ambil waktu CPU ---
        snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
        fp = fopen(path, "r");
        if (!fp) continue;

        // /proc/[pid]/stat memiliki banyak field, kita ambil field ke-14 dan ke-15 (utime dan stime)
        // Format: pid (comm) state ppid ... utime stime ...
        // Kita perlu melewati tanda kurung dulu
        int dummy;
        char comm[256], state;
        fscanf(fp, "%d %s %c", &dummy, comm, &state);
        for (int i = 0; i < 11; i++) fscanf(fp, "%*s"); // skip sampai ke utime

        fscanf(fp, "%ld %ld", &utime, &stime);
        fclose(fp);
        total_time = utime + stime;

        double cpu_usage = (double)total_time / ticks_per_sec;

        printf("PID: %s | Cmd: %-15s | RAM: %5lu KB | CPU: %.2f s\n",
               entry->d_name, name, vsize, cpu_usage);
    }

    closedir(proc);
}
```
<p>Fungsi list_processes(const char *username) ini digunakan untuk menampilkan daftar proses milik user tertentu, lengkap dengan informasi seperti PID, nama proses, penggunaan RAM, dan waktu CPU.</p>
<p>Direktori /proc berisi subdirektori untuk setiap proses yang sedang berjalan, dengan nama berupa angka PID.</p>
<p>Fungsi get_uid() adalah fungsi buatan yang mengembalikan UID (User ID) dari nama pengguna.
Ini digunakan untuk menyaring hanya proses milik user tersebut.</p>

```c
while ((entry = readdir(proc)) != NULL) {
    if (!isdigit(entry->d_name[0])) continue;
snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
```
<p>Pada bagian ini program akan meLooping setiap entri di /proc dan Hanya entri yang nama filenya angka (artinya PID) yang diproses, serta memprint outputnya.</p>

```c
snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
fscanf(fp, "%d %s %c", &dummy, comm, &state);
// Skip sampai field ke-14
for (int i = 0; i < 11; i++) fscanf(fp, "%*s");
fscanf(fp, "%ld %ld", &utime, &stime);
double cpu_usage = (double)total_time / ticks_per_sec;
```
<p> Pada bagain ini program akan mengambil informasi menngenai CPU melalui Field ke-14 (utime) dan ke-15 (stime) yang mana itu adalah waktu CPU dalam clock ticks.
Keduanya dijumlahkan menjadi total_time.</p>

```c
printf("PID: %s | Cmd: %-15s | RAM: %5lu KB | CPU: %.2f s\n",
       entry->d_name, name, vsize, cpu_usage);
```
<p>Print keseluruhan informasi.<br>
nanti hasil outputnya seperti ini
</p>
<img src="https://github.com/user-attachments/assets/dd05aa1b-4c1b-4c6e-b021-b019ecbc0eab">


<h3>4B.</h3>

```c
void run_daemon(const char *username) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) {
        printf("Debugmon daemon started (PID: %d)\n", pid);
        FILE *f = fopen(PID_FILE, "w");
        if (f) {
            fprintf(f, "%d", pid);
            fclose(f);
        }
        exit(0);
    }

    setsid(); // detach from terminal

    uid_t uid = get_uid(username);

    while (1) {
        DIR *proc = opendir("/proc");
        if (!proc) continue;

        struct dirent *entry;
        while ((entry = readdir(proc)) != NULL) {
            if (!isdigit(entry->d_name[0])) continue;

            char path[256], name[100] = "";
            uid_t proc_uid = -1;
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);

            FILE *fp = fopen(path, "r");
            if (!fp) continue;

            char line[256];
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "Name:", 5) == 0)
                    sscanf(line, "Name:\t%99s", name);
                if (strncmp(line, "Uid:", 4) == 0) {
                    sscanf(line, "Uid:\t%d", &proc_uid);
                    break;
                }
            }
            fclose(fp);

            if (proc_uid == uid) {
                write_log(name, "RUNNING");
            }
        }

        closedir(proc);
        sleep(5);
    }
}
```
<p>Fungsi ini menjalankan daemon bernama "Debugmon", yang bertugas memantau proses milik user tertentu dan mencatatnya ke log jika sedang berjalan.</p>

```c
pid_t pid = fork();
if (pid < 0) exit(EXIT_FAILURE);         // Jika gagal fork, keluar dengan error
if (pid > 0) {
    printf("Debugmon daemon started (PID: %d)\n", pid);
    FILE *f = fopen(PID_FILE, "w");      // Simpan PID child ke file PID_FILE
    if (f) {
        fprintf(f, "%d", pid);
        fclose(f);
    }
    exit(0);                             // Proses parent keluar
}
```

<p>Fungsi fork() memisahkan proses menjadi parent dan child.
Parent mencetak PID dan menyimpan ke file, lalu keluar.
Child lanjut berjalan sebagai daemon. </p>

```c
while (1) {
    DIR *proc = opendir("/proc");
    if (!proc) continue;
while ((entry = readdir(proc)) != NULL) {
    if (!isdigit(entry->d_name[0])) continue;
```

<p>Pada bagian ini program akan embuka direktori /proc yang berisi info semua proses di sistem Linux.
Jika gagal buka, lanjut ke iterasi berikutnya.
Setelah itu program akan mengiterasi Proses dalam /proc.</p>

```c
char path[256], name[100] = "";
uid_t proc_uid = -1;
snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);

FILE *fp = fopen(path, "r");
if (!fp) continue;

char line[256];
while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "Name:", 5) == 0)
        sscanf(line, "Name:\t%99s", name);
    if (strncmp(line, "Uid:", 4) == 0) {
        sscanf(line, "Uid:\t%d", &proc_uid);
        break;
    }
}
fclose(fp);
```
<p>Pada bagian ini program akan membuka file /proc/<pid>/status untuk baca informasi proses, lalu mengambil informasi <br>
Name: nama program/proses. <br>
Uid: UID dari pemilik proses.</p>

<h3>4C.</h3>
