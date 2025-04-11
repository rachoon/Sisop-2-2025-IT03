#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>

#define LOG_FILE "debugmon.log"
#define PID_FILE "/tmp/debugmon_daemon.pid"
#define FAIL_FLAG "/tmp/debugmon_failed.flag"

void write_log(const char *proc_name, const char *status) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(log, "[%02d:%02d:%04d-%02d:%02d:%02d]_%s_%s\n",
        tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        proc_name, status);

    fclose(log);
}

uid_t get_uid(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (!pw) {
        fprintf(stderr, "User not found: %s\n", username);
        exit(EXIT_FAILURE);
    }
    return pw->pw_uid;
}

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

void stop_daemon() {
    FILE *f = fopen(PID_FILE, "r");
    if (!f) {
        printf("No daemon is running.\n");
        return;
    }
    int pid;
    fscanf(f, "%d", &pid);
    fclose(f);
    kill(pid, SIGTERM);
    unlink(PID_FILE);
    printf("Daemon stopped.\n");
}

void fail_user(const char *username) {
    uid_t uid = get_uid(username);
    FILE *flag = fopen(FAIL_FLAG, "w");
    if (flag) fclose(flag);

    DIR *proc = opendir("/proc");
    if (!proc) return;

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
            int pid = atoi(entry->d_name);
            kill(pid, SIGKILL);
            write_log(name, "FAILED");
        }
    }

    closedir(proc);
    printf("All processes of %s terminated.\n", username);
}

void revert_user(const char *username) {
    unlink(FAIL_FLAG);
    printf("User %s can run processes again.\n", username);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  %s list <user>\n", argv[0]);
        printf("  %s daemon <user>\n", argv[0]);
        printf("  %s stop <user>\n", argv[0]);
        printf("  %s fail <user>\n", argv[0]);
        printf("  %s revert <user>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        list_processes(argv[2]);
    } else if (strcmp(argv[1], "daemon") == 0) {
        run_daemon(argv[2]);
    } else if (strcmp(argv[1], "stop") == 0) {
        stop_daemon();
    } else if (strcmp(argv[1], "fail") == 0) {
        fail_user(argv[2]);
    } else if (strcmp(argv[1], "revert") == 0) {
        revert_user(argv[2]);
    } else {
        printf("Unknown command.\n");
    }

    return 0;
}
