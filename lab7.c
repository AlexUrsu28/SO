#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <dirent.h>
#define MAX 4096
char buffer[MAX];
char out[MAX];

typedef struct
{
    uint16_t signature;  // 2B
    uint32_t fileSize;   // 4B
    uint16_t reserved1;  // 2B
    uint16_t reserved2;  // 2B
    uint32_t dataOffset; // 4B
    uint32_t headerSize; // 4B
    int32_t width;       // 4B
    int32_t height;      // 4B
    uint16_t planes;
    uint16_t bitDepth;
} Header;

struct timespec st_mtim; /* time of last modification */
Header header;
struct stat st;
struct dirent *entry;

int afiseaza_tipul(const char *nume_fisier)
{
    if (stat(nume_fisier, &st) == 0)
    {
        if (S_ISREG(st.st_mode))
        {
            printf("%s este un fisier obisnuit\n", nume_fisier);
            return 1;
        }
        else if (S_ISDIR(st.st_mode))
        {
            printf("%s este un director\n", nume_fisier);
            return 2;
        }
        else if (S_ISLNK(st.st_mode))
        {
            printf("%s este o legatura simbolica\n", nume_fisier);
            return 3;
        }
    }
    else
    {
        perror("Eroare la obtinerea informatiilor despre fisier");
    }
    return 0;
}

void extractHeader(int fd)
{
    memcpy(&header.width, buffer + 18, 4);
    memcpy(&header.height, buffer + 22, 4);
    sprintf(out, "lungime : %d , inaltime : %d\n", header.width, header.height);
    write(fd, out, strlen(out));
}

void extractData(int fd, const char *nume_fisier)
{
    printf("Numele fisierului este: %s\n", nume_fisier);
    if (stat(nume_fisier, &st) == 0)
    {
        int length = 0;
        length = st.st_size;
        int id = 0;
        id = st.st_uid;
        int contor = 0;
        contor = st.st_nlink;
        // Utilizează st_mtime pentru a obține timpul ultimei modificări
        time_t ultima_modificare = st.st_mtime;
        // Converteste timpul in struct tm
        struct tm *tm_info = localtime(&ultima_modificare);
        sprintf(out, "Dimensiunea fisierului este: %d\n", length);
        write(fd, out, strlen(out));
        sprintf(out, "id : %d\n", id);
        write(fd, out, strlen(out));
        sprintf(out, "Numar de legaturi : %d\n", contor);
        write(fd, out, strlen(out));
        sprintf(out, "Timpul ultimei modificari: %s", asctime(tm_info));
        write(fd, out, strlen(out));
    }
    else
    {
        printf("Eroare la obtinerea informatiilor despre fisier");
    }
}

void extractDataSymLink(int fd, char *nume_fisier)
{
    if (stat(nume_fisier, &st) == 0)
    {
        int length = 0;
        length = st.st_size;
        sprintf(out, "Dimensiunea fisierului target este: %d\n", length);
        write(fd, out, strlen(out));
    }
    else if (lstat(nume_fisier, &st) == 0)
    {
        int length = 0;
        length = st.st_size;
        sprintf(out, "Dimensiunea fisierului symlink este: %d\n", length);
        write(fd, out, strlen(out));
    }
    else
    {
        perror("Eroare la obtinerea informatiilor despre symlink");
    }
}

void extractDrepturi(mode_t mode, int fd)
{
    // Permisiuni pentru utilizator
    sprintf(out, "Utilizator: \n ");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IRUSR) ? "r" : "-");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IWUSR) ? "w" : "-");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IXUSR) ? "x" : "-");
    write(fd, out, strlen(out));
    write(fd, "\n", strlen("\n"));

    // Permisiuni pentru grup
    sprintf(out, "Grup: ");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IRGRP) ? "r" : "-");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IWGRP) ? "w" : "-");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IXGRP) ? "x" : "-");
    write(fd, out, strlen(out));
    write(fd, "\n", strlen("\n"));

    // Permisiuni pentru alți utilizatori
    sprintf(out, "Alți utilizatori: ");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IROTH) ? "r" : "-");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IWOTH) ? "w" : "-");
    write(fd, out, strlen(out));
    sprintf(out, (mode & S_IXOTH) ? "x" : "-");
    write(fd, out, strlen(out));
    write(fd, "\n", strlen("\n"));
}

int main(int args, char *argv[])
{
    DIR *dir;
    const char *path = argv[1];
    dir = opendir(path);
    if (dir == NULL)
    {
        printf(out, "Nu exista directorul\n");
        return 0;
    }
    //aici trebuie inchis la fiecare compilare fisietul ststistica deoarece trebuie creat mai intai
    int file2 = open("statistica.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (file2 == -1)
    {
        sprintf(out, "Fisierul nu exista\n");
        write(file2, out, strlen(out));
        return 0;
    }
    entry = readdir(dir);
    while (entry != NULL)
    {
        char file_path[MAX];
        strcpy(file_path, path);
        strcat(file_path, "/");
        strcat(file_path, entry->d_name);
        strcat(file_path, "\0");
        if (afiseaza_tipul(file_path) == 1)
        {
            int file_desc = open(file_path, O_RDONLY);
            if (file_desc == -1)
            {
                printf(out, "Fisierul nu exista\n");
            }
            int nr1 = read(file_desc, buffer, MAX);
            if (nr1 == -1)
            {
                printf("Eroare la citire\n");
            }
            memcpy(&header.signature, buffer, 2);
            if (header.signature == 0x4D42)
            {
                sprintf(out, "Fișierul %s este de tip BMP.\n", entry->d_name);
                write(file2, out, strlen(out));
                extractData(file2, file_path);
                extractHeader(file2);
                if (stat(file_path, &st) == 0)
                {
                    extractDrepturi(st.st_mode, file2);
                }
            }
            else
            {
                sprintf(out, "Numele fisierului obisnuit: %s\n", entry->d_name);
                write(file2, out, strlen(out));
                extractData(file2, file_path);
                if (stat(file_path, &st) == 0)
                {
                    extractDrepturi(st.st_mode, file2);
                }
            }
        }
        else if (afiseaza_tipul(file_path) == 2)
        {
            sprintf(out, "Numele directorului este: %s\n", entry->d_name);
            write(file2, out, strlen(out));
            if (stat(file_path, &st) == 0)
            {
                int user = st.st_uid;
                sprintf(out, "id : %d\n", user);
                write(file2, out, strlen(out));
                extractDrepturi(st.st_mode, file2);
            }
        }
        else if (afiseaza_tipul(file_path) == 3)
        {
            sprintf(out, "Numele legaturii este: %s\n", entry->d_name);
            write(file2, out, strlen(out));
            extractDataSymLink(file2, file_path);
            if (lstat(file_path, &st) == 0)
            {
                extractDrepturi(st.st_mode, file2);
            }
        }
        entry = readdir(dir);
    }
    if (close(file2) == -1)
    {
        sprintf(out, "Fisierul nu a fost inchis\n");
        write(file2, out, strlen(out));
        return 0;
    }
    if(closedir(dir)==-1)
    {
        sprintf(out, "Directorul nu a fost inchis\n");
        write(file2, out, strlen(out));
        return 0;
    }
    return 0;
}