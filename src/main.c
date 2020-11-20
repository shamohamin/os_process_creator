#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "main.h"

int main()
{
    Command *ptr;
    ptr = (Command *)malloc(sizeof(Command));

    char *file_path = "command.txt";
    reading_file(file_path, ptr);

    creating_process(ptr);
}

void creating_process(Command *command_st)
{
    int buff_size = LINE_SIZE * command_st->child_proccess_count;
    char write_msg[buff_size];
    char write_msg_child[buff_size];
    char read_msg_child[buff_size];
    int pipe_buffer_size = 0;
    char read_msg[buff_size];
    char omadn[LINE_SIZE];
    childs = (ChildInfo **)malloc(
        sizeof(ChildInfo *) * command_st->commands_count);
    for (int i = 0; i < command_st->commands_count; i++)
        childs[i] = NULL;
    clock_t start = clock();
    pid_t main_routing_id = getpid();

    int fd[2];

    if (pipe(fd) == -1)
        ERROR_HANDLER_AND_DIE("couldn't make pipe;");

    for (int i = 0; i < command_st->proccess_count; i++)
    {
        memset(write_msg, 0, buff_size);

        pid_t pid = fork();
        if (pid < 0)
        {
            ERROR_HANDLER_AND_DIE("cant make the process.");
        }
        else if (pid == 0) // child
        {
            int fd_child[2];
            pid_t id_of_process[command_st->child_proccess_count];

            if (pipe(fd_child) == -1)
                ERROR_HANDLER_AND_DIE("couldn't make pipe;");

            read(fd[READ_END], read_msg, sizeof(write_msg));

            for (int j = 0; j < command_st->child_proccess_count; j++)
            {
                memset(write_msg_child, 0, buff_size);

                pid_t child_pid = fork();

                if (child_pid < 0)
                {
                    ERROR_HANDLER_AND_DIE("cant make the childs process.");
                }
                else if (child_pid == 0) // child of child
                {
                    child_process(read_msg_child, fd_child, buff_size);
                }
                strcpy(
                    write_msg_child,
                    split_the_generated_commnad(read_msg, j, command_st->child_proccess_count));

                write(fd_child[WRITE_END], write_msg_child, buff_size);
            }

            for (int j = 0; j < command_st->child_proccess_count; j++)
            {
                wait(NULL);
            }

            exit(0);
        }

        command_st->commands_count = enque(
            create_new_process_info(pid, main_routing_id, IS_NOT_CHILD, IS_PARRENT, i + 1, -1),
            command_st->commands_count);

        int start_index = i * command_st->child_proccess_count;
        int end_index = start_index + command_st->child_proccess_count;
        strcpy(write_msg, generating_commands(start_index, end_index, command_st));
        write(fd[WRITE_END], write_msg, buff_size);
    }

    for (int i = 0; i < command_st->proccess_count; i++)
    {
        int status;
        pid_t child_id = wait(&status);
        set_end_time_and_status_for_terminated_process(child_id, status, command_st->commands_count);
        // read(fd[READ_END], omadn, strlen(write_  msg));
        // printf("\n%s\n", omadn);
    }

    for (int i = 0; childs[i] != NULL; i++)
        printing_process_info(childs[i]);

    
    SHOWING_CONSUMING_TIME((double)(clock() - start) / CLOCKS_PER_SEC);
}

void child_process(char read_msg_child[], int fd_child[], int buff_size)
{
    memset(read_msg_child, 0, buff_size);
    read(fd_child[READ_END], read_msg_child, buff_size);
    char **args = malloc(sizeof(char *) * MAX_ARGS_LEN);
    int args_len = parse_command_to_be_executed(read_msg_child, args);
    int time_of_command = (int)strtol(args[args_len], NULL, 10);
    args[args_len] = NULL;

    if ((execv(args[0], &args[1]) < 0))
    {
        printf("\n warning!!!!!!!: couldnt execute!. command: %s\n", read_msg_child);
        exit(0);
    }
}

ChildInfo *create_new_process_info(
    pid_t process_id,
    pid_t parrent_id,
    int is_child,
    int is_parrent,
    int process_number,
    int parrent_number)
{
    ChildInfo *c_process = (ChildInfo *)malloc(sizeof(ChildInfo));
    if (c_process == NULL)
        ERROR_HANDLER_AND_DIE("Allocating wasn't successfull;");

    c_process->id = process_id;
    c_process->parrent_id = parrent_id;
    c_process->is_parrent = is_parrent;
    c_process->start_time = clock();
    c_process->is_child = is_child;
    c_process->process_number = process_number;
    c_process->parrent_number = parrent_number;

    return c_process;
}

// mimic the queue for queue of childs
int enque(ChildInfo *c_process, int MAX_QUEUE_SIZE)
{
    int count = 0;

    for (int i = 0; i < MAX_QUEUE_SIZE; i++, count++)
    {
        if (childs[i] == NULL)
            break;
    }

    // reallocating
    if (count > MAX_QUEUE_SIZE)
    {
        childs = (ChildInfo **)realloc(childs, (sizeof(ChildInfo *) * count));
        for (int i = MAX_QUEUE_SIZE; i < count; i++)
            childs[i] = NULL;
    }

    if (!childs)
        ERROR_HANDLER_AND_DIE("reallocating wasn't sucessfull!!!!");

    childs[count] = c_process;

    return count > MAX_QUEUE_SIZE ? count : MAX_QUEUE_SIZE;
}

void set_end_time_and_status_for_terminated_process(pid_t pid, int exit_status, int MAX_QUEUE_SIZE)
{
    int found = 0;
    for (int i = 0; i < MAX_QUEUE_SIZE; i++)
    {
        if (childs[i]->id == pid)
        {
            found = 1;
            childs[i]->end_time = clock();
            childs[i]->execution_time = (((double)(childs[i]->end_time - childs[i]->start_time)) / CLOCKS_PER_SEC);
            childs[i]->exit_status = exit_status;
            break;
        }
    }

    if (!found)
        ERROR_HANDLER_AND_DIE("Couldn't find the process.");
}