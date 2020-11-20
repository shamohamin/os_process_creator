#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef _MAIN_H
#define _MAIN_H

#define IS_PARRENT 1
#define IS_CHILD 1
#define IS_NOT_CHILD -1
#define IS_NOT_PARRENT -1
#define SIZE 1000
#define LINE_SIZE 100
#define MAX_ARGS_LEN 30
#define READ_END 0
#define WRITE_END 1

#define ERROR_HANDLER_AND_DIE(err) \
    {                              \
        fprintf(stderr, err);      \
        exit(1);                   \
    }

#define SHOWING_CONSUMING_TIME(elapsed_time)                                           \
    {                                                                                  \
        printf("***********************\n");                                           \
        printf("TIMING OF PROGRAM \n");                                                \
        printf("time in seconds: %lf\n", ((double)elapsed_time));                      \
        printf("time in milli seconds: %lf ms\n", ((double)elapsed_time) * 1000.0);    \
        printf("time in micro seconds: %lf us\n", ((double)elapsed_time) * 1000000.0); \
        printf("***********************\n");                                           \
    }

typedef struct
{
    int proccess_count;
    int child_proccess_count;
    char *commands[SIZE];
    int commands_count;
} Command;

typedef struct
{
    pid_t parrent_id;
    pid_t id;
    clock_t start_time;
    clock_t end_time;
    double execution_time;
    int is_parrent;
    int is_child;
    int process_number;
    int parrent_number;
    int exit_status;
} ChildInfo;

static ChildInfo **childs;

void reading_file(char *, Command *);
char *line_convertion(char *);
char *seperating_values(char *, int);
void creating_process(Command *);
char *generating_commands(int, int, Command *);
char *split_the_generated_commnad(char[], int, int);
int parse_command_to_be_executed(char *, char **);
void child_process(char[], int[], int);
ChildInfo *create_new_process_info(pid_t, pid_t, int, int, int, int);
int enque(ChildInfo *, int);
void set_end_time_and_status_for_terminated_process(pid_t, int, int);
void printing_process_info(ChildInfo *c_process);

#endif