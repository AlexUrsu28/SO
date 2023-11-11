#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
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

int main(int argc, char *argv[])
{
    int file1 = open(argv[1], O_RDONLY);
    if (file1 == -1) 
    {
        printf(out,"Fisierul nu exista\n");
        return 0;
    }
    int nr1 = read(file1, buffer, MAX);
    if (nr1 == -1) 
    {
        printf("Eroare la citire\n");
        return 0;
    }
    int file2 = open("statistica.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (file2 == -1) 
    {
        printf("Fisierul nu exista\n");
        return 0;
    }
    Header header;
    memcpy(&header.signature, buffer, 2);
    memcpy(&header.fileSize, buffer + 2, 4);
    memcpy(&header.reserved1, buffer + 6, 2);
    memcpy(&header.reserved2, buffer + 8, 2);
    memcpy(&header.dataOffset, buffer + 10, 4);
    memcpy(&header.headerSize, buffer + 14, 4);
    memcpy(&header.width, buffer + 18, 4);
    memcpy(&header.height, buffer + 22, 4);
    memcpy(&header.planes, buffer + 26, 2);
    memcpy(&header.bitDepth, buffer + 28, 2);
    if(header.signature == 0x4D42)
    {
        printf("Fișierul %s este de tip BMP.\n", argv[1]);
    }
    struct stat st;
    if (stat(argv[1], &st) == 0)
    {
       int length = 0;
       length = st.st_size; 
       int id = 0;
       id = st.st_uid;
       int contor = 0;
       contor = st.st_nlink;
       sprintf(out,"Dimensiunea fisierului este: %d\n",length);
       write(file2,out,strlen(out));
       sprintf(out, "lungime : %d , inaltime : %d\n", header.width, header.height);
       write(file2,out,strlen(out));
       sprintf(out, "nume fisier : %s\n", argv[1]);
       write(file2,out,strlen(out));
       sprintf(out, "id : %d\n", id);
       write(file2,out,strlen(out));
       sprintf(out,"Numar de legaturi : %d\n",contor);
       write(file2,out,strlen(out));
    }
    else 
    {
        printf("Eroare la stat\n");
        return 0;
    }
    return 0;
}