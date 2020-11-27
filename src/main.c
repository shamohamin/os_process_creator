#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "headers.h"

int main()
{
    char *file_path = "command.txt";
    start_execution(file_path);
}

void start_execution(char *file_path)
{
    Command *ptr;
    ptr = (Command *)malloc(sizeof(Command));

    reading_file(file_path, ptr);
    ptr->generated_configuration = (int *)malloc(sizeof(int) * ptr->commands_count);
    ptr->number_of_divsor = generate_divisors_of_number(
        ptr->commands_count, ptr->generated_configuration);

    ProcessConfigurations **holder = (ProcessConfigurations **)malloc(
        ptr->number_of_divsor * sizeof(ProcessConfigurations *));

    for (int i = 0; i < ptr->number_of_divsor; i++)
    {
        holder[i] = (ProcessConfigurations *)malloc(sizeof(ProcessConfigurations));
        int temp = ptr->generated_configuration[i];
        ptr->child_proccess_count = temp;
        ptr->proccess_count = ptr->commands_count / temp;
        holder[i]->configuration[0] = temp;
        holder[i]->configuration[1] = ptr->commands_count / temp;
        struct timeval start, end;
        gettimeofday(&start, NULL);
        creating_process(ptr, holder[i]);
        printf("configuration$$$ parent level1-> %d $$$ child_process-> %d\n",
               holder[i]->configuration[1], holder[i]->configuration[0]);
        gettimeofday(&end, NULL);
        holder[i]->end = end;
        holder[i]->start = start;
        holder[i]->execution_time = COMPUTE_EXECUTION_TIME(start, end);
        SHOWING_CONSUMING_TIME(holder[i]->execution_time, "TIMING OF PROGRAM ");
    }
    write_output_file(ptr->number_of_divsor, holder);
}

void write_output_file(int size, ProcessConfigurations **holder)
{
    FILE *out = fopen("output.json", "w");
    fputs("[\n", out);

    for (int i = 0; i < size; i++)
    {
        fputs("\t{\n", out);
        holder[i]->execution_time = 1000.0 * holder[i]->execution_time;
        put_line_in_file(out, "\t\t\"%s\":%lf,\n", "execution time", &holder[i]->execution_time, DOUBLE);
        fputs("\t\t\"configuration\":[\n \t\t\t{\n", out);
        put_line_in_file(out, "\t\t\t\t\"%s\":%d, \n", "number of process level 1", &holder[i]->configuration[1], INT);
        put_line_in_file(out, "\t\t\t\t\"%s\":%d \n", "number of process level 2", &holder[i]->configuration[0], INT);
        fputs("\t\t\t}\n\t\t],\n", out);
        fputs("\t\t\"process created\":[\n", out);
        for (int j = 0; j < (holder[i]->childs_size); j++)
        {
            fputs("\t\t{\n", out);
            put_line_in_file(out, "\t\t\t\"%s\":%lf, \n", "execution time",
                             &holder[i]->process_created_inconfiguration[j]->execution_time, DOUBLE);
            put_line_in_file(out, "\t\t\t\"%s\":\"%s\", \n", "Command",
                             holder[i]->process_created_inconfiguration[j]->command, STRING);
            put_line_in_file(out, "\t\t\t\"%s\":%d, \n", "exit status",
                             &holder[i]->process_created_inconfiguration[j]->exit_status, INT);
            put_line_in_file(out, "\t\t\t\"%s\":%d, \n", "id",
                             &holder[i]->process_created_inconfiguration[j]->id, INT);
            put_line_in_file(out, "\t\t\t\"%s\":%d, \n", "parrent_id",
                             &holder[i]->process_created_inconfiguration[j]->parrent_id, INT);
            put_line_in_file(out, "\t\t\t\"%s\":%d, \n", "child process number",
                             &holder[i]->process_created_inconfiguration[j]->process_number, INT);
            put_line_in_file(out, "\t\t\t\"%s\":%d \n", "parrent process number",
                             &holder[i]->process_created_inconfiguration[j]->parrent_number, INT);
            put_line_in_file(out, "\t\t\t\"%s\":%d \n", "number of trys",
                             &holder[i]->process_created_inconfiguration[j]->number_of_trys, INT);
            if (j == holder[i]->childs_size - 1)
                fputs("\t\t}\n", out);
            else
                fputs("\t\t},\n", out);
        }
        fputs("\t\t]\n", out);
        if (i == size - 1)
            fputs("\t}\n", out);
        else
            fputs("\t},\n", out);
    }
    fputs("]\n", out);
    fclose(out);
}

void put_line_in_file(FILE *file, char *format, char *key, void *value, int op)
{
    char *tmp = (char *)malloc(sizeof(char) * LINE_SIZE);
    switch (op)
    {
    case DOUBLE:
        sprintf(tmp, format, key, *(double *)value);
        break;
    case INT:
        sprintf(tmp, format, key, *(int *)value);
        break;
    case STRING:
    {
        char *t = (char *)value;
        int c = strlen(t);
        t[c - 1] = '\0';
        sprintf(tmp, format, key, t);
        free(t);
    }
    default:
        break;
    }

    fputs(tmp, file);
    free(tmp);
}