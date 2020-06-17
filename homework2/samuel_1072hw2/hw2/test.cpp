#include <iostream>
#include <string>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>

int main()
{
    dup2(2, 1);
    fprintf(stdout, "stdout\n");

    char str[10] = {'1', '2', '3', '\n', '5', '6', '7', '8', '9', '\0'};
    int fd = creat("test.txt", 00777);
    pwrite(fd, str, 10, 0);
    close(fd);

    char str2[10] = {'\0'};
    fd = open("test.txt", 0);
    pread(fd, str2, 3, 0);
    fprintf(stdout, "%s", str2);

    FILE* file = fdopen(fd, "r");
    fclose(file);
    
    fwrite(str, 1, 10, stdout);

    file = fopen("test.txt", "r");
    fprintf(stdout, "%c", fgetc(file));
    fprintf(stdout, "%s", fgets(str2, 10, file));
    fprintf(stdout, "%s", fgets(str2, 10, file));
    fprintf(stdout, "%c", fgetc(file));

    int a;
    fscanf(stdin, "%d", &a);
    chdir("../hw2");
    chown("a.so", -1, -1);
    chmod("a.so", 00777);
    rename("test.txt", "a.txt");
    link("a.txt", "b.txt");
    exit(0);
    readlink("b.txt", str2, 10);
    fprintf(stdout, "%s", str2);
    unlink("b.txt");
    symlink("a.txt", "b.txt");
    unlink("b.txt");
    remove("a.txt");
    mkdir("123", 00777);
    rmdir("123");
}
