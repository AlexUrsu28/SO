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
#include <sys/wait.h>

#define MAX 4096
char buffer[MAX];
char out[MAX];
int count_process = 0;
int pfd[2];
char *path_out;
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
        perror("Eroare la obtinerea informatiilor despre fisier");
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

void countLines(char *array)
{
    for (int i = 0; i < strlen(array); i++)
    {
        if (array[i] == ' ')
        {
            array[i] = '\0';
            break;
        }
    }
    printf("Numarul de linii in fisier este: %s\n", array);
}

/*int createFileinDir(char *name_file)
{
    char aux[MAX] = {0};
    strcpy(aux, path_out);
    printf("\n\nname = %s\n\n", name_file);
    printf("\n\naux = %s\n\n", aux);
    strcat(aux, name_file);
    strcat(aux, "\0");
    printf("\n\naux = %s\n\n", aux);
    int fd = open(aux, O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd == -1)
    {
        perror("Fisierul nu exista\n");
        return 0;
    }
    return fd;
}?*/

char *concatFormat(char *name_file)
{
    char aux[MAX] = {0};
    strcat(aux, name_file);
    strcat(aux, "_statistica.txt");
    strcat(aux, "\0");
    char *aux2 = aux;
    return aux2;
}

void waitPID(int count_process)
{
    for (int i = 0; i < count_process; i++)
    {
        int status;
        int w;
        w = wait(&status);
        if (w == -1)
        {
            perror(" Eroare la waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
        {
            printf("The process with PID %d has ended with the exit code %d\n", w, WEXITSTATUS(status));
        }
    }
}

int main(int args, char *argv[])
{
    DIR *dir;
    const char *path = argv[1];
    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Nu exista directorul\n");
        return 0;
    }
    DIR *dir_out;
    path_out = argv[2];
    dir_out = opendir(path_out);
    if (dir_out == NULL)
    {
        perror("Nu exista directorul\n");
        return 0;
    }
    // aici trebuie inchis la fiecare compilare fisietul ststistica deoarece trebuie creat mai intai
    int file2 = open("statistica.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (file2 == -1)
    {
        perror("Fisierul nu exista\n");
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
                perror("Fisierul nu exista\n");
                return 0;
            }
            int nr1 = read(file_desc, buffer, MAX);
            if (nr1 == -1)
            {
                perror("Eroare la citire\n");
                return 0;
            }
            memcpy(&header.signature, buffer, 2);
            if (header.signature == 0x4D42)
            {
                if (pipe(pfd) < 0)
                {
                    perror("Eroare la pipe\n");
                    return 0;
                }
                pid_t pid = fork();
                if (pid == -1)
                {
                    perror("Eroare la fork\n");
                    return 0;
                }
                count_process++;
                if (pid == 0)
                {
                    // copil
                    close(pfd[0]); // inchid capat de citire
                    char *file_name = concatFormat(entry->d_name);
                    char aux[MAX] = {0};
                    strcpy(aux, path_out);
                    strcat(aux, file_name);
                    strcat(aux, "\0");
                    int fd = open(aux, O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
                    if (fd == -1)
                    {
                        perror("Fisierul nu exista\n");
                        return 0;
                    }
                    extractData(fd, file_path);
                    extractHeader(fd);
                    if (stat(file_path, &st) == 0)
                    {
                        extractDrepturi(st.st_mode, fd);
                    }
                    // Redirecționează ieșirea standard către pipe
                    dup2(pfd[1], 1);
                    execlp("wc", "wc", "-l", aux , NULL);
                }
                // parinte
                close(pfd[1]); // inchid capat de scriere
                char array[MAX];
                read(pfd[0], array, MAX);
                printf("Numarul de linii in fisier este: %s\n", array);
                countLines(array);
                sprintf(out,"Numărul de linii în fișier: %s\n", array);
                write(file2, out, strlen(out));
                close(pfd[0]);
            }
            else
            {
                if (pipe(pfd) < 0)
                {
                    perror("Eroare la pipe\n");
                    return 0;
                }
                pid_t pid = fork();
                if (pid == -1)
                {
                    perror("Eroare la fork\n");
                    return 0;
                }
                count_process++;
                if (pid == 0)
                {
                    // copil
                    close(pfd[0]); // inchid capat de citire
                    char *file_name = concatFormat(entry->d_name);
                    char aux[MAX] = {0};
                    strcpy(aux, path_out);
                    strcat(aux, file_name);
                    strcat(aux, "\0");
                    int fd = open(aux, O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
                    if (fd == -1)
                    {
                        perror("Fisierul nu exista\n");
                        return 0;
                    }
                    extractData(fd, file_path);
                    if (stat(file_path, &st) == 0)
                    {
                        extractDrepturi(st.st_mode, fd);
                    }
                    // Redirecționează ieșirea standard către pipe
                    dup2(pfd[1], 1);
                    execlp("wc", "wc", "-l", aux , NULL);
                }
                // parinte
                close(pfd[1]); // inchid capat de scriere
                char array[MAX] ={0};
                read(pfd[0], array, MAX);
                countLines(array);
                sprintf(out,"Numărul de linii în fișier: %s\n", array);
                write(file2, out, strlen(out));
                close(pfd[0]);
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
            int file_desc = open(file_path, O_RDONLY);
            if (file_desc == -1)
            {
                perror("Fisierul nu exista\n");
                return 0;
            }
            int nr1 = read(file_desc, buffer, MAX);
            if (nr1 == -1)
            {
                perror("Eroare la citire\n");
                return 0;
            }
            if (pipe(pfd) < 0)
                {
                    perror("Eroare la pipe\n");
                    return 0;
                }
                pid_t pid = fork();
                if (pid == -1)
                {
                    perror("Eroare la fork\n");
                    return 0;
                }
                count_process++;
                if (pid == 0)
                {
                    // copil
                    close(pfd[0]); // inchid capat de citire
                    char *file_name = concatFormat(entry->d_name);
                    char aux[MAX] = {0};
                    strcpy(aux, path_out);
                    strcat(aux, file_name);
                    strcat(aux, "\0");
                    int fd = open(aux, O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
                    if (fd == -1)
                    {
                        perror("Fisierul nu exista\n");
                        return 0;
                    }
                    sprintf(out, "Numele legaturii este: %s\n", entry->d_name);
                    write(fd, out, strlen(out));
                    extractDataSymLink(fd, file_path);
                    if (lstat(file_path, &st) == 0)
                    {
                        extractDrepturi(st.st_mode, fd);
                    }
                }
                // parinte
                close(pfd[1]); // inchid capat de scriere
                char array[MAX] ={0};
                read(pfd[0], array, MAX);
                countLines(array);
                sprintf(out,"Numărul de linii în fișier: %s\n", array);
                write(file2, out, strlen(out));
                close(pfd[0]);
        }
        entry = readdir(dir);
    }
    if (closedir(dir_out) == -1)
    {
        perror("Directorul out nu a fost inchis\n");
        return 0;
    }
    if (closedir(dir) == -1)
    {
        perror("Directorul input nu a fost inchis\n");
        return 0;
    }
    waitPID(count_process);
    return 0;
}