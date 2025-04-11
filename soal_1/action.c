#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

void download()
{
    if (!opendir("Clues.zip"))
    {
        pid_t pid_download = fork();
        if (pid_download == 0)
        {
            char *download = "wget -q 'https://drive.usercontent.google.com/download?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download&authuser=0' -O Clues.zip";
            char *argv_download[] = {"sh", "-c", download, NULL};
            execv("/bin/sh", argv_download);
            
            char *unzip = "sudo unzip -q Clues.zip";
            char *argv_unzip[] = {"sh", "-c", unzip, NULL};
            execv("/bin/sh", argv_unzip);

            char *rm = "rm Clues.zip";
            char *argv_rm[] = {"sh", "-c", rm, NULL};
            execv("/bin/sh", argv_rm);
        }
        else if (pid_download > 0) wait(NULL);
        else perror("fork");
    }
    printf("Usage: ./action -m [Filter|Combine|Decode]\n");

}

int check_filter(const char *name) {
    return strlen(name) == 5 && (isalpha(name[0]) || isdigit(name[0]));
}

void filter()
{
    
    mkdir("Filtered", 0755);
    DIR *d;
    struct dirent *dir;
    char path[256];
    
    for (int i = 0; i < 4; i++)
    {
        sprintf(path, "Clues/Clue%c", 'A'+i);
        d = opendir(path);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;

                // inisialisasi full path dari semua folder Clues
                char full_path[512];
                sprintf(full_path, "%s/%s", path, dir->d_name);

                if (check_filter(dir->d_name)) {
                    // Move yang sudah difilter ke folder /Filtered
                    pid_t pid = fork();
                    if (pid == 0) {
                        char *argv[] = {"mv", full_path, "Filtered/", NULL};
                        execv("/bin/mv", argv);
                        perror("mv failed");
                        exit(EXIT_FAILURE);
                    } else wait(NULL);
                } else {
                    // hapus file yang tidak terfilter
                    pid_t pid = fork();
                    if (pid == 0) {
                        char *argv[] = {"rm", "-f", full_path, NULL};
                        execv("/bin/rm", argv);
                        perror("rm failed");
                        exit(EXIT_FAILURE);
                    } else wait(NULL);
                }
            }
            closedir(d);
        }
    }
}

void combine()
{
    FILE *output = fopen("Combined.txt", "w");


    for (int i = 1; i <= 26; i++)
    {
        // file angka
        char file_angka[256];
        sprintf(file_angka, "Filtered/%d.txt", i);

        FILE *file = fopen(file_angka, "r");
        if (file)
        {
            char c;
            if (fscanf(file, " %c", &c) == 1)
            {
            fprintf(output, "%c", c);
            }
            fclose(file);
        }   
        // file huruf
        char file_huruf[256];
        // check file yang dimaksud kosong atau tidak
        if (i <= 26) 
        {
            sprintf(file_huruf, "Filtered/%c.txt", 'a'+ i - 1);
        }
        else
        {
            file_huruf[0] = '\0';
        }
        fopen(file_huruf, "r");
        if (file)
        {
            char c;
            if (fscanf(file, "%c", &c) == 1)
            {
            fprintf(output, "%c", c);
            }
            fclose(file);
        }  
    }

    fclose(output);
}

void ROT13(char *rot)
{
    for (; *rot; rot++)
    {
        if (isalpha(*rot))
        {
            *rot = (toupper(*rot) <= 'M') ? *rot + 13 : *rot - 13;
        }
    }
}

void decode()
{
    // membuka file Decoding.txt
    FILE *combine = fopen("Combined.txt", "r");
    if (!combine) return;

    // Copy isinya ke rot
    char rot[256];
    fscanf(combine, "%s", rot);
    fclose(combine);

    // Melakukan encoding ROT13
    ROT13(rot);

    // Salin hasil ROT13 ke Decoded.txt
    FILE *output = fopen("Decoded.txt", "w");
    fprintf(output, "%s", rot);
    fclose(output);


}

int main(int argc, char *argv[])
{
    if (argc != 3 || strcmp(argv[1], "-m"))
    {
        download();
        return 1;

    }

    if (!strcmp(argv[2], "Filter")) filter();
    else if (!strcmp(argv[2], "Combine")) combine();
    else if (!strcmp(argv[2], "Decode")) decode();
    else 
    {
        printf("%s not found. \n\nUsage: %s -m [Filter|Combine|Decode]\n", argv[2], argv[0]);
    };
}

