#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "headers.h"

void creating_process(Command *command_st)
{
    int all_childs_count = command_st->commands_count + command_st->proccess_count;
    int buff_size = LINE_SIZE * command_st->child_proccess_count;
    char write_msg[buff_size];
    char write_msg_child[buff_size];
    char read_msg_child[buff_size];
    int pipe_buffer_size = 0;
    char read_msg[buff_size];
    ChildInfo created_process_reader[command_st->child_proccess_count];
    childs = (ChildInfo **)malloc(
        sizeof(ChildInfo *) * (all_childs_count));
    for (int i = 0; i < (all_childs_count); i++)
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
            ChildInfo created_process_write[command_st->child_proccess_count];

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

                created_process_write[j] = create_new_process_info(
                    child_pid, getpid(), IS_CHILD, IS_NOT_PARRENT, j + 1, i + 1);

                write(fd_child[WRITE_END], write_msg_child, buff_size);
            }

            for (int j = 0; j < command_st->child_proccess_count; j++)
            {
                int status;
                pid_t __child_id = wait(&status);
                set_end_time_and_status_for_terminated_process(
                    NULL, created_process_write, __child_id, status, command_st->child_proccess_count);
            }

            ssize_t byets_written = write(
                fd[WRITE_END], created_process_write,
                sizeof(ChildInfo) * command_st->child_proccess_count);
            printf("\nwritten bytes : %zd\n", byets_written);
            exit(0);
        }
        ChildInfo dummy;
        all_childs_count = enqueue(
            childs,
            create_new_process_ptr_info(pid, main_routing_id, IS_NOT_CHILD, IS_PARRENT, i + 1, -1),
            dummy,
            all_childs_count);

        int start_index = i * command_st->child_proccess_count;
        int end_index = start_index + command_st->child_proccess_count;
        strcpy(write_msg, generating_commands(start_index, end_index, command_st));
        write(fd[WRITE_END], write_msg, buff_size);
    }

    handeling_wait_for_proccess(
        command_st, command_st->proccess_count,
        IS_PARRENT,
        fd,
        created_process_reader,
        childs, &all_childs_count);

    for (int i = 0; childs[i] != NULL && i < (all_childs_count); i++)
        printing_process_info(childs[i]);

    SHOWING_CONSUMING_TIME((double)(clock() - start) / CLOCKS_PER_SEC, "TIMING OF PROGRAM ");
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
        char *err = (char *)malloc(sizeof(char) * LINE_SIZE);
        sprintf(err, "warning!!!!!!!: couldnt execute!. command: %s", read_msg_child);
        ERROR_HANDLER_AND_DIE(err);
    }
}

ChildInfo *create_new_process_ptr_info(
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

ChildInfo create_new_process_info(
    pid_t process_id,
    pid_t parrent_id,
    int is_child,
    int is_parrent,
    int process_number,
    int parrent_number)
{
    ChildInfo c_process;

    c_process.id = process_id;
    c_process.parrent_id = parrent_id;
    c_process.is_parrent = is_parrent;
    c_process.start_time = clock();
    c_process.is_child = is_child;
    c_process.process_number = process_number;
    c_process.parrent_number = parrent_number;

    return c_process;
}

/* 
  mimic the queue for queue of childs
    return the allocated size of new Holder
*/
int enqueue(ChildInfo **holder, ChildInfo *c_process_ptr, ChildInfo c_process, int MAX_QUEUE_SIZE)
{
    int count = 0;

    for (int i = 0; i < MAX_QUEUE_SIZE; i++, count++)
    {
        if (holder[i] == NULL)
            break;
    }

    // reallocating
    if (count >= MAX_QUEUE_SIZE)
    {
        holder = (ChildInfo **)realloc(holder, (sizeof(ChildInfo *) * count));
        if (!holder)
            ERROR_HANDLER_AND_DIE("reallocating wasn't sucessfull!!!!");
        for (int i = MAX_QUEUE_SIZE; i < count; i++)
            holder[i] = NULL;
    }

    if (c_process_ptr == NULL)
    {
        holder[count] = create_new_process_ptr_info(c_process.id,
                                                    c_process.parrent_id,
                                                    c_process.is_child,
                                                    c_process.is_parrent,
                                                    c_process.process_number,
                                                    c_process.parrent_number);
        holder[count]->end_time = c_process.end_time;
        holder[count]->execution_time = c_process.execution_time;
        holder[count]->exit_status = c_process.exit_status;
    }
    else
    {
        holder[count] = c_process_ptr;
    }

    return count > MAX_QUEUE_SIZE ? count : MAX_QUEUE_SIZE;
}

void set_end_time_and_status_for_terminated_process(
    ChildInfo **holder,
    ChildInfo holder_1[],
    pid_t pid,
    int exit_status,
    int MAX_QUEUE_SIZE)
{
    int found = 0;
    if (holder != NULL)
    {
        for (int i = 0; i < MAX_QUEUE_SIZE; i++)
        {
            if (holder[i]->id == pid)
            {
                found = 1;
                holder[i]->end_time = clock();
                holder[i]->execution_time = (((double)(holder[i]->end_time - holder[i]->start_time)) / CLOCKS_PER_SEC);
                holder[i]->exit_status = exit_status;
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < MAX_QUEUE_SIZE; i++)
        {
            if (holder_1[i].id == pid)
            {
                found = 1;
                holder_1[i].end_time = clock();
                holder_1[i].execution_time = (((double)(holder_1[i].end_time - holder_1[i].start_time)) / CLOCKS_PER_SEC);
                holder_1[i].exit_status = exit_status;
                break;
            }
        }
    }

    if (!found)
        ERROR_HANDLER_AND_DIE("Couldn't find the process.");
}

void handeling_wait_for_proccess(
    Command *command_st,
    int counter,
    int is_parrent,
    int fd[],
    ChildInfo childs_reader[],
    ChildInfo **holder,
    int *all_childs_count)
{
    for (int i = 0; i < counter; i++)
    {
        memset(childs_reader, 0, sizeof(ChildInfo) * command_st->child_proccess_count);
        int status;
        pid_t child_id = wait(&status);
        set_end_time_and_status_for_terminated_process(holder,
                                                       NULL, child_id, status, command_st->commands_count);
        ssize_t readed_bytes_size = read(
            fd[READ_END],
            childs_reader,
            sizeof(ChildInfo) * command_st->child_proccess_count);
        printf("\n readed bytes: %zd \n", readed_bytes_size);
        if (readed_bytes_size > 0)
            for (int j = 0; j < command_st->child_proccess_count; j++)
                *all_childs_count = enqueue(childs, NULL, childs_reader[j], *all_childs_count);
    }
}