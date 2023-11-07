#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

#define MAX 4096

int main(int argc, char *argv[]) 
{
    int file1 = open(argv[1], O_RDONLY);
    if (file1 == -1) 
    {
        printf("Fisierul nu exista\n");
        return 0;
    }
    int file2 = open(argv[2], O_WRONLY);
    if (file2 == -1) 
    {
        printf("Fisierul nu exista\n");
        return 0;
    }
    char buffer[MAX];
    int nr1 = read(file1, buffer, MAX);
    if (nr1 == -1) 
    {
        printf("Eroare la citire\n");
        return 0;
    }
    int count_upper = 0;
    int count_lower = 0;
    int number = 0;
    int count = 0;
    int length = 0;
    for (int i = 0; i < nr1; i++) 
    {
        if (isupper(buffer[i])) 
        {
            count_upper++;
        }
        if (islower(buffer[i])) 
        {
            count_lower++;
        }
        if (isdigit(buffer[i])) 
        {
            number++;
        }
        if(argv[3][0] == buffer[i])
        {
            count++;
        }
    }
    char out[MAX];
    if (close(file1) == -1) 
    {
        printf("Eroare la inchidere\n");
        return 0;
    }
    int l = 0;
    struct stat st;
    if (stat(argv[1], &st) == 0)
    {
        l = st.st_size;
    }
    else 
    {
        printf("Eroare la stat\n");
        return 0;
    }
    sprintf (out,"Numar litere mici %d \n", count_lower);
    write(file2,out,strlen(out));
    sprintf (out,"Numar litere mari %d \n", count_upper);
    write(file2,out,strlen(out));
    sprintf (out,"Numar cifre %d \n", number);
    write(file2,out,strlen(out));
    sprintf(out,"Numar aparitii %d \n", count);
    write(file2,out,strlen(out));
    sprintf(out,"Dimensiune fisier %d \n", l);
    write(file2,out,strlen(out));
    return 0;
}