#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#define MAX 4096
char buffer[MAX];
char out[MAX];

typedef struct {
    uint16_t signature; //2B
    uint32_t fileSize;  //4B
    uint16_t reserved1; //2B
    uint16_t reserved2; //2B
    uint32_t dataOffset; //4B
    uint32_t headerSize; //4B
    int32_t width; //4B
    int32_t height; //4B
    uint16_t planes;
    uint16_t bitDepth;
}Header;

struct timespec st_mtim;  /* time of last modification */
Header header;
struct stat st;

void verificareInput()
{
    int file1 = open("img.bmp", O_RDONLY);
    if (file1 == -1) 
    {
        printf(out,"Fisierul nu exista\n");
    }
    int nr1 = read(file1, buffer, MAX);
    if (nr1 == -1) 
    {
        printf("Eroare la citire\n");
    }
}

void extractHeader(int fd)
{
    memcpy(&header.width, buffer + 18, 4);
    memcpy(&header.height, buffer + 22, 4);
    sprintf(out, "lungime : %d , inaltime : %d\n", header.width, header.height);
    write(fd,out,strlen(out));
}

void extractData(int fd)
{
    const char *nume_fisier = "img.bmp";
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
       sprintf(out,"Dimensiunea fisierului este: %d\n",length);
       write(fd,out,strlen(out));
       sprintf(out, "id : %d\n", id);
       write(fd,out,strlen(out));
       sprintf(out,"Numar de legaturi : %d\n",contor);
       write(fd,out,strlen(out));
       sprintf(out, "Timpul ultimei modificari: %s", asctime(tm_info));
       write(fd,out,strlen(out));
    }
    else {
        perror("Eroare la obtinerea informatiilor despre fisier");
    }
}

void extractDrepturi(mode_t mode,int fd) 
{
    // Permisiuni pentru utilizator
    sprintf(out, "Utilizator: \n ");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IRUSR) ? "r" : "-");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IWUSR) ? "w" : "-");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IXUSR) ? "x" : "-");
    write(fd,out,strlen(out));
    write(fd,"\n",strlen("\n"));

    // Permisiuni pentru grup
    sprintf(out, "Grup: ");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IRGRP) ? "r" : "-");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IWGRP) ? "w" : "-");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IXGRP) ? "x" : "-");
    write(fd,out,strlen(out));
    write(fd,"\n",strlen("\n"));

    // Permisiuni pentru alți utilizatori
    sprintf(out, "Alți utilizatori: ");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IROTH) ? "r" : "-");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IWOTH) ? "w" : "-");
    write(fd,out,strlen(out));
    sprintf(out, (mode & S_IXOTH) ? "x" : "-");
    write(fd,out,strlen(out));
    write(fd,"\n",strlen("\n"));
}

 int main(int args,char *argv[])
 {
    verificareInput();
    memcpy(&header.signature, buffer, 2);
    if(header.signature == 0x4D42 && args == 2)
    {
        printf("Fișierul %s este de tip BMP.\n", argv[1]);
    }else 
    {
        printf(" Eroare \n");
    }
    int file2 = open("statistica.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (file2 == -1) 
    {
        printf("Fisierul nu exista\n");
    }
    sprintf(out, "Numele fisierului este: %s\n", argv[1]);
    write(file2,out,strlen(out));
    extractHeader(file2);
    extractData(file2);
    const char *nume_fisier = "img.bmp";
    if (stat(nume_fisier, &st) == 0) {
        extractDrepturi(st.st_mode,file2);
    } else {
        perror("Eroare la obtinerea informatiilor despre fisier");
    }
    return 0;
 }