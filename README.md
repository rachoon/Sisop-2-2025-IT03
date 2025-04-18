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

// mendefinisikan proses execvp dan fork
void run_process(char *args[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp gagal");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork gagal");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}
```

<p>
Fungsi tersebut untuk mendownload dan unzip file yang sudah di download. Seteleha file berhasil didonwload, file zip akan otomatis terhabus. Fungsi tersebut juga bisa untuk mengekstrak isi file tersebut ke dalam file starter kit. <br>

Output yang akan dihasilkan adalah 

```c
Mengunduh file dari Google Drive...
Mengekstrak starterkit.zip ke folder starter_kit...
Selesai!
```

  <h3>Membuat directory karantina yang dapat mendecrypt nama file menggunakan algoritma base64</h3>

```c
// memeriksa apakah ada string yang valid dalam base64 
int is_base64(const char *str) {
    while (*str) {
        if (!(isalnum(*str) || *str == '+' || *str == '/' || *str == '='))
            return 0;
        str++;
    }
    return 1;
}

// decode base64 ke bentuk asli
char *base64_decode(const char *data) {
    char command[512];
    snprintf(command, sizeof(command), "echo '%s' | base64 -d", data);
    FILE *fp = popen(command, "r");
    if (!fp) return NULL;

    char *decoded = malloc(256);
    if (!decoded) return NULL;

    if (fgets(decoded, 256, fp) == NULL) {
        pclose(fp);
        free(decoded);
        return NULL;
    }
    pclose(fp);

    decoded[strcspn(decoded, "\n")] = 0;
    return decoded;
}

// decrypt file
void daemon_decrypt() {
    struct stat st = {0};
    if (stat("quarantine", &st) == -1) {
        mkdir("quarantine", 0700);
    }

    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) {
        FILE *fp = fopen("daemon.pid", "w");
        if (fp) {
            fprintf(fp, "%d\n", pid);
            fclose(fp);
            write_log("Successfully started decryption process with PID %d.", pid);
        }
        exit(EXIT_SUCCESS);
    }

    umask(0);
    setsid();
    chdir(".");
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    while (1) {
        DIR *d = opendir("quarantine");
        if (!d) {
            sleep(5);
            continue;
        }

        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG && is_base64(dir->d_name)) {
                char oldname[512], newname[512];
                snprintf(oldname, sizeof(oldname), "quarantine/%s", dir->d_name);
                char *decoded = base64_decode(dir->d_name);
                if (decoded && strlen(decoded) > 0) {
                    snprintf(newname, sizeof(newname), "quarantine/%s", decoded);
                    rename(oldname, newname);
                }
                free(decoded);
            }
        }
        closedir(d);
        sleep(5);
    }
```

<p>
Fungsi dari kode tersebut adalah decode nama file dan menecek apakah ada string yang terdiri dari base64 valid. Fungsi ini juga untuk membuat fie bernama "quarantine". file bernama "daemon.pid" untuk melacak PID daemon.

  <h3>Memindahkan file yang ada pada directory starter kit ke karantina dan sebaliknya</h3>

```c
void move_to_quarantine() {
    struct stat st = {0};
    if (stat("quarantine", &st) == -1) {
        mkdir("quarantine", 0700);
    }

    DIR *d = opendir("starter_kit");
    if (!d) {
        perror("starter_kit tidak ditemukan");
        return;
    }

    struct dirent *dir;
    char src[512], dest[512];
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            snprintf(src, sizeof(src), "starter_kit/%s", dir->d_name);
            snprintf(dest, sizeof(dest), "quarantine/%s", dir->d_name);
            if (rename(src, dest) == 0) {
                write_log("%s - Successfully moved to quarantine directory.", dir->d_name);
            }
        }
    }
    closedir(d);
    printf("Semua file dari starter_kit dipindah ke quarantine.\n");
}

// memindahkan file dari quarantine ke starter kit 
void return_files() {
    DIR *d = opendir("quarantine");
    if (!d) {
        perror("quarantine tidak ditemukan");
        return;
    }

    struct dirent *dir;
    char src[512], dest[512];
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            snprintf(src, sizeof(src), "quarantine/%s", dir->d_name);
            snprintf(dest, sizeof(dest), "starter_kit/%s", dir->d_name);
            if (rename(src, dest) == 0) {
                write_log("%s - Successfully returned to starter kit directory.", dir->d_name);
            }
        }
    }
    closedir(d);
    printf("Semua file telah dikembalikan ke starter_kit.\n");
}
```

<p>
Fungsi dari kode tersebut adalah untuk memindahkan semua file yang ada di "starter_kit" ke folder "quarantine". Kode tersebut juga berfungsi untuk mengembalikkan semua file yang ada di folder "quarantine" ke folder "starter_kit" kembali. <br>

  Output dari kode tersebut jika berhasil memindahkan file ke "quarantine" adalah

```c
Semua file dari starter_kit dipindah ke quarantine.
```

Output dari kode tersebut jika berhasil mengembalikkan file ke "starter_kit" adalah

```c
Semua file telah dikembalikan ke starter_kit.
```
  

  <h3>Menghapus seluruh file yang ada pada directory karantina</h3>
  
```c
void eradicate_quarantine() {
    DIR *d = opendir("quarantine");
    if (!d) {
        perror("quarantine tidak ditemukan");
        return;
    }

    struct dirent *dir;
    char filepath[512];
    int deleted = 0;

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            snprintf(filepath, sizeof(filepath), "quarantine/%s", dir->d_name);
            if (remove(filepath) == 0) {
                deleted++;
                write_log("%s - Successfully deleted.", dir->d_name);
            }
        }
    }
    closedir(d);

    printf("%d file berhasil dihapus dari quarantine.\n", deleted);
}
```

<p>
Fungsi dari kode tersebut adalah menghapus file yang ada di folder "quarantine" menggunakan opendir(). <br>

Output yang dihasilkan ketika gagal menghapus file adalah 

```c
quarantine tidak ditemukan
```

<p>
Output yang dihasilkan jika berhasil menghapus file dari "quarantine" adalah

```c
file berhasil dihapus dari quarantine
```
  
  <h3>Mematikan program decrypt secara aman berdasarkan PID dri proses tersebut</h3>

```c
void shutdown_daemon() {
    FILE *fp = fopen("daemon.pid", "r");
    if (!fp) {
        fprintf(stderr, "Tidak dapat menemukan daemon.pid (mungkin daemon tidak berjalan?)\n");
        return;
    }

    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        fprintf(stderr, "Gagal membaca PID dari file.\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        printf("Proses daemon dengan PID %d berhasil dihentikan.\n", pid);
        write_log("Successfully shut off decryption process with PID %d.", pid);
        remove("daemon.pid");
    } else {
        perror("Gagal menghentikan proses daemon");
    }
}
```

<p>
File ini berisi PID dari proses daemon yang dijalankan sebelumnya. <br>

Ouput yang dihasilkan jika gagal mematikan program adalah

```c
Tidak dapat menemukan daemon.pid (mungkin daemon tidak berjalan?)
```

<p>
Output yang dihasilkan jika berhasil mematikan program adalah

```c
Proses daemon dengan PID ... berhasil dihentikan.
```

  <h3>Membuat error handling</h3>

```c
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("\n\033[1;31m ERROR:\033[0m Anda harus memberikan argumen yang valid.\n");
        printf("Penggunaan: %s [--download | --decrypt | --quarantine | --return | --eradicate | --shutdown]\n\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--download") == 0) {
        download_and_unzip();
    } else if (strcmp(argv[1], "--decrypt") == 0) {
        printf("Menyalakan daemon decrypt di folder quarantine/...\n");
        daemon_decrypt();
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        printf("Memindahkan file ke quarantine...\n");
        move_to_quarantine();
    } else if (strcmp(argv[1], "--return") == 0) {
        return_files();
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_quarantine();
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_daemon();
    } else {
        printf("\n\033[1;31m ERROR:\033[0m Argumen tidak dikenal: %s\n", argv[1]);
        printf("Silakan gunakan salah satu dari: --download | --decrypt | --quarantine | --return | --eradicate | --shutdown\n\n");
        return 1;
    }

    return 0;
}
```

<p>
Fungsi dari kode tersebut adalah memberikan petunjuk untuk menjalan program yang ada. <br>

  Output yang akan terjadi adalah 

```c
ERROR: Anda harus memberikan argumen yang valid.
Penggunaan: ./starterkit [--download | --decrypt | --quarantine | --return | --eradicate | --shutdown]
```

  <h3>mencatat penggunaan program dan log dari setiap penggunaan, lalu menyimpannya ke dalam file bernama activity.log</h3>

```c
// mencatat program log activity
void write_log(const char *format, ...) {
    FILE *logfile = fopen("activity.log", "a");
    if (!logfile) return;

    // mencatat setiap penggunaan program ke activity.log
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(logfile, "[%02d-%02d-%04d][%02d:%02d:%02d] - ",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
            
    // menulis log dengan format yang ada
    va_list args;
    va_start(args, format);
    vfprintf(logfile, format, args);
    va_end(args);

    fprintf(logfile, "\n");
    fclose(logfile);
}
```

<p>
Fungsi dari kode tersebut adalah mencatat aktivitas program setiap penggunaan ke dalam file "activity.log". <br>

Contoh dari isi dari activity adalah 

```c
[18-04-2025][22:23:09] - cGFzc3dvcmRfc3RlYWxlci5leGUK - Successfully moved to quarantine directory.
[18-04-2025][22:23:09] - project_plan.docx - Successfully moved to quarantine directory.
[18-04-2025][22:23:09] - dHJvamFuLmV4ZQo= - Successfully moved to quarantine directory.
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
