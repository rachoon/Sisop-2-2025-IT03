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
<p>Print keseluruhan informasi.</p>
