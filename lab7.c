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
#include <regex.h>

// #define REGEX_PATTERN "^[A-Z][A-Za-z0-9\, ]*\.$"
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

typedef struct
{
    uint8_t red;   // 1B
    uint8_t green; // 1B
    uint8_t blue;  // 1B
} Pixel;

Pixel pixel;

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

char *concatFormat(char *name_file)
{
    char aux[MAX] = {0};
    strcat(aux, name_file);
    strcat(aux, "_statistica.txt");
    strcat(aux, "\0");
    char *aux2 = aux;
    return aux2;
}

int parcurgereToregex(char *array, char caracter)
{
    regex_t regex1;
    regex_t regex2;
    if (regcomp(&regex1, "^[A-Z][A-Za-z0-9 ,]*[A-Za-z0-9]*[.!?]$", REG_EXTENDED) != 0)
    {
        perror("Eroare la compilarea expresiei regulate.\n");
        return 0;
    }
    if (regcomp(&regex2, "si[ ],", REG_EXTENDED) != 0)
    {
        perror("Eroare la compilarea expresiei regulate.\n");
        return 0;
    }
    int matching_propositions = 0;
    char *p = strtok(array, "\n");
    while (p)
    {
        if ((regexec(&regex1, p, 0, NULL, 0) == 0) && (strchr(p, caracter) != NULL))
        {
            if((regexec(&regex2, p, 0, NULL, 0)) != 0)
            {
                matching_propositions++;
            }
        }
        p = strtok(NULL, "\n");
    }
    return matching_propositions;
}

void colorTOgri(char *nume_fisier)
{
    int fd = open(nume_fisier, O_RDWR);
    if (fd == -1)
    {
        perror("Fisierul nu exista\n");
        exit(1);
    }
    int nr = read(fd, buffer, 70);
    if (nr == -1)
    {
        perror("Eroare la citire\n");
        exit(1);
    }
    memcpy(&header.width, buffer + 18, 4);
    memcpy(&header.height, buffer + 22, 4);
    // printf("height %d width %d\n", header.height, header.width);
    memcpy(&pixel.red, buffer + 54, 1);
    memcpy(&pixel.green, buffer + 55, 1);
    memcpy(&pixel.blue, buffer + 56, 1);
    uint8_t greyValue = (uint8_t)(0.299 * pixel.red + 0.587 * pixel.green + 0.114 * pixel.blue);
    for (int i = 0; i < header.height; i++)
    {
        for (int j = 0; j < header.width; j++)
        {
            // Citirea pixelilor
            if (read(fd, &pixel.red, sizeof(pixel.red)) == -1)
            {
                perror("Eroare la citire\n");
                exit(1);
            }
            if (read(fd, &pixel.green, sizeof(pixel.green)) == -1)
            {
                perror("Eroare la citire\n");
                exit(1);
            }
            if (read(fd, &pixel.blue, sizeof(pixel.blue)) == -1)
            {
                perror("Eroare la citire\n");
                exit(1);
            }
            // printf("%d %d %d\n", pixel.red, pixel.green, pixel.blue);
            greyValue = (uint8_t)(0.299 * pixel.red + 0.587 * pixel.green + 0.114 * pixel.blue);
            // Setarea cursorului în poziția corectă pentru scrierea pixelului în același loc
            if (lseek(fd, -3, SEEK_CUR) == -1)
            {
                perror("Eroare la lseek\n");
                exit(1);
            }

            // Scrierea pixelului în același loc
            if (write(fd, &greyValue, sizeof(pixel.red)) == -1)
            {
                perror("Eroare la scriere\n");
                exit(1);
            }
            if (write(fd, &greyValue, sizeof(pixel.green)) == -1)
            {
                perror("Eroare la scriere\n");
                exit(1);
            }
            if (write(fd, &greyValue, sizeof(pixel.blue)) == -1)
            {
                perror("Eroare la scriere\n");
                exit(1);
            }
        }
    }
    // printf("height %d width %d\n", header.height, header.width);
    close(fd);
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
                    exit(1);
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
                    execlp("wc", "wc", "-l", aux, NULL);
                }
                else
                {
                    // parinte
                    close(pfd[1]); // inchid capat de scriere
                    char array[MAX];
                    read(pfd[0], array, MAX);
                    printf("Numarul de linii in fisier este: %s\n", array);
                    countLines(array);
                    sprintf(out, "Numărul de linii în fișier: %s\n", array);
                    write(file2, out, strlen(out));
                    close(pfd[0]);
                    pid_t pid2 = fork();
                    if (pid2 == -1)
                    {
                        perror("Eroare la al 2-lea fork\n");
                        return 0;
                    }
                    count_process++;
                    if (pid2 == 0)
                    {
                        // copil 2
                        colorTOgri(file_path);
                        exit(0);
                    }
                }
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
                    exit(1);
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
                    printf("                                        Auxiliarul este: %s\n", aux);
                    dup2(pfd[1], 1);
                    execlp("cat", "cat", file_path, NULL);
                }
                // parinte
                close(pfd[1]); // inchid capat de scriere
                char array[MAX] = {0};
                read(pfd[0], array, MAX);
                printf("                                        Arrayul este: %s\n sfarsitul arrayului \n", array);
                close(pfd[0]);
                pid_t pid2 = fork();
                if (pid2 == -1)
                {
                    perror("Eroare la al 2-lea fork\n");
                    return 0;
                }
                count_process++;
                if (pid2 == 0)
                {
                    // copil 2
                    close(pfd[0]); // inchid capat de citire
                    dup2(pfd[1], 1);
                    // int contor_proposition = parcurgereToregex(array, argv[3][0]);
                    // dup2(pfd[1], 1);
                    // printf("Numarul de propozitii corecte este: %d care contin caracterul: %c \n", contor_proposition, argv[3][0]);
                    execlp("sh", "sh", "lab4+.sh", argv[3][0], NULL);
                    exit(0);
                }
                else
                {
                    // parinte
                    close(pfd[1]); // inchid capat de scriere
                    char array[MAX] = {0};
                    read(pfd[0], array, MAX);
                    array[MAX - 1] = '\0';
                    printf("%s\n", array);
                    close(pfd[0]);
                }
            }
        }
        else if (afiseaza_tipul(file_path) == 2)
        {
            DIR *dir_test;
            dir_test = opendir(file_path);
            if (dir_test == NULL)
            {
                perror("Nu exista directorul\n");
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
                exit(1);
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
                sprintf(out, "Numele directorului este: %s\n", entry->d_name);
                write(fd, out, strlen(out));
                if (stat(file_path, &st) == 0)
                {
                    int user = st.st_uid;
                    sprintf(out, "id : %d\n", user);
                    write(fd, out, strlen(out));
                    extractDrepturi(st.st_mode, fd);
                }
                dup2(pfd[1], 1);
                execlp("wc", "wc", "-l", aux, NULL);
            }
            else
            {
                // parinte
                close(pfd[1]); // inchid capat de scriere
                char array[MAX];
                read(pfd[0], array, MAX);
                printf("Numarul de linii in fisier este: %s\n", array);
                countLines(array);
                sprintf(out, "Numărul de linii în fișier: %s\n", array);
                write(file2, out, strlen(out));
                close(pfd[0]);
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
                // Redirecționează ieșirea standard către pipe
                dup2(pfd[1], 1);
                execlp("wc", "wc", "-l", aux, NULL);
            }
            else
            {
                // parinte
                close(pfd[1]); // inchid capat de scriere
                char array[MAX] = {0};
                read(pfd[0], array, MAX);
                countLines(array);
                sprintf(out, "Numărul de linii în fișier: %s\n", array);
                write(file2, out, strlen(out));
                close(pfd[0]);
            }
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