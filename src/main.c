#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "headers.h"

int main()
{
    Command *ptr;
    ptr = (Command *)malloc(sizeof(Command));

    char *file_path = "command.txt";
    reading_file(file_path, ptr);

    creating_process(ptr);
}