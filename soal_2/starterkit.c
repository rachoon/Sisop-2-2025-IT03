#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>

// Fungsi mencatat log
void write_log(const char *format, ...) {
    FILE *logfile = fopen("activity.log", "a");
    if (!logfile) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(logfile, "[%02d-%02d-%04d][%02d:%02d:%02d] - ",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);

    va_list args;
    va_start(args, format);
    vfprintf(logfile, format, args);
    va_end(args);

    fprintf(logfile, "\n");
    fclose(logfile);
}

// Fungsi untuk mengunduh dan unzip
void download_and_unzip() {
    printf("\U0001F4E5 Mengunduh file dari Google Drive...\n");
    system("wget -O starterkit.zip 'https://drive.usercontent.google.com/download?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&confirm=t' > /dev/null 2>&1");

    struct stat st = {0};
    if (stat("starter_kit", &st) == -1) {
        mkdir("starter_kit", 0700);
    }

    printf("\U0001F4E6 Mengekstrak starterkit.zip ke folder starter_kit...\n");
    system("unzip -q starterkit.zip -d starter_kit");
    remove("starterkit.zip");
    printf("✅ Selesai!\n");
}

int is_base64(const char *str) {
    while (*str) {
        if (!(isalnum(*str) || *str == '+' || *str == '/' || *str == '='))
            return 0;
        str++;
    }
    return 1;
}

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
    printf("✅ Semua file dari starter_kit dipindah ke quarantine.\n");
}

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
}

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
    printf("✅ Semua file telah dikembalikan ke starter_kit.\n");
}

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

    printf("\U0001F9F9 %d file berhasil dihapus dari quarantine.\n", deleted);
}

void shutdown_daemon() {
    FILE *fp = fopen("daemon.pid", "r");
    if (!fp) {
        fprintf(stderr, "⚠️  Tidak dapat menemukan daemon.pid (mungkin daemon tidak berjalan?)\n");
        return;
    }

    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        fprintf(stderr, "❌ Gagal membaca PID dari file.\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        printf("🛑 Proses daemon dengan PID %d berhasil dihentikan.\n", pid);
        write_log("Successfully shut off decryption process with PID %d.", pid);
        remove("daemon.pid");
    } else {
        perror("❌ Gagal menghentikan proses daemon");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("\n\033[1;31m⚠️  ERROR:\033[0m Anda harus memberikan argumen yang valid.\n");
        printf("Penggunaan: %s [--download | --decrypt | --quarantine | --return | --eradicate | --shutdown]\n\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--download") == 0) {
        download_and_unzip();
    } else if (strcmp(argv[1], "--decrypt") == 0) {
        printf("\U0001F510 Menyalakan daemon decrypt di folder quarantine/...\n");
        daemon_decrypt();
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        printf("\U0001F501 Memindahkan file ke quarantine...\n");
        move_to_quarantine();
    } else if (strcmp(argv[1], "--return") == 0) {
        return_files();
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_quarantine();
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_daemon();
    } else {
        printf("\n\033[1;31m⚠️  ERROR:\033[0m Argumen tidak dikenal: %s\n", argv[1]);
        printf("Silakan gunakan salah satu dari: --download | --decrypt | --quarantine | --return | --eradicate | --shutdown\n\n");
        return 1;
    }

    return 0;
}
