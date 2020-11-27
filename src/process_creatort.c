#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "headers.h"

void creating_process(Command *command_st, ProcessConfigurations *conf)
{
    int all_childs_count = command_st->commands_count + command_st->proccess_count;
    int buff_size = LINE_SIZE * 5;
    char write_msg[buff_size];
    char write_msg_child[buff_size];
    char read_msg_child[buff_size];
    int pipe_buffer_size = 0;
    char read_msg[buff_size];
    ChildInfo created_process_reader[command_st->child_proccess_count];
    ChildInfo *childs[all_childs_count + 1];
    for (int i = 0; i < (all_childs_count + 1); i++)
    {
        childs[i] = (ChildInfo *)malloc(sizeof(ChildInfo));
        childs[i]->id = ENQUEU_ID;
    }
    int counter = 0;

    int fd[2 * command_st->proccess_count];
    printf("ptr : %d\n", command_st->proccess_count);
    pipe_creator(fd, command_st->proccess_count);
    pid_t children_id[command_st->proccess_count];
    pid_t main_routing_id = getpid();

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
            pid_t process_childs[command_st->child_proccess_count];
            ChildInfo created_process_write[command_st->child_proccess_count];

            if (pipe(fd_child) == -1)
                ERROR_HANDLER_AND_DIE("couldn't make pipe;");

            ssize_t sizzzz = read(fd[2 * i + READ_END], read_msg, buff_size);

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
                int t = 0;
                pid_t __child_id = wait(&status);
                waitpid(process_childs[j], &status, 0);
                // retry
                if (status != 0)
                {
                    pid_t ppid = fork();
                    if (ppid == 0)
                    {
                        child_process(read_msg_child, fd_child, buff_size);
                    }
                    strcpy(write_msg_child,
                           split_the_generated_commnad(read_msg, j, command_st->child_proccess_count));
                    write(fd_child[WRITE_END], write_msg_child, buff_size);
                    wait(NULL);
                    t = 1;
                }
                set_end_time_and_status_for_terminated_process(
                    NULL, created_process_write, process_childs[j],
                    -status, command_st->child_proccess_count, t);
            }

            ssize_t byets_written = write(
                fd[2 * i + WRITE_END], created_process_write,
                sizeof(ChildInfo) * command_st->child_proccess_count);
            exit(0);
        }
        children_id[i] = pid;
        ChildInfo dummy;
        all_childs_count = enqueue(
            childs,
            create_new_process_ptr_info(pid, main_routing_id, IS_NOT_CHILD, IS_PARRENT, i + 1, -1),
            dummy,
            all_childs_count, &counter);

        int start_index = i * command_st->child_proccess_count;
        int end_index = start_index + command_st->child_proccess_count;
        strcpy(write_msg, generating_commands(start_index, end_index, command_st));
        ssize_t written = write(fd[2 * i + WRITE_END], write_msg, sizeof(write_msg));
    }

    handeling_wait_for_proccess(
        command_st, command_st->proccess_count,
        IS_PARRENT,
        fd,
        created_process_reader,
        childs, &all_childs_count,
        children_id, childs, &counter);

    copy_process(all_childs_count, conf, childs, command_st);
    for (int i = 0; i < command_st->proccess_count; i++)
        -close(fd[2 * i]);
}

void child_process(char read_msg_child[], int fd_child[], int buff_size)
{
    memset(read_msg_child, 0, buff_size);
    read(fd_child[READ_END], read_msg_child, buff_size);
    char **args = malloc(sizeof(char *) * MAX_ARGS_LEN);
    int args_len = parse_command_to_be_executed(read_msg_child, args);
    long time_of_command = strtol(args[args_len], NULL, 10);
    usleep(time_of_command * 1000);
    args[args_len] = NULL;

    if ((execv(args[0], &args[1]) < 0))
    {
        char *err = (char *)malloc(sizeof(char) * LINE_SIZE);
        sprintf(err, "warning!!!!!!!: couldnt execute!. command: %s", read_msg_child);
        ERROR_HANDLER_AND_DIE(err);
    }
    exit(0);
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
    gettimeofday(&c_process->start_time, NULL);
    c_process->is_child = is_child;
    c_process->process_number = process_number;
    c_process->parrent_number = parrent_number;
    c_process->command = (char *)malloc(1 * LINE_SIZE);
    c_process->number_of_trys = 1;
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
    gettimeofday(&c_process.start_time, NULL);
    c_process.is_child = is_child;
    c_process.process_number = process_number;
    c_process.parrent_number = parrent_number;
    c_process.command = (char *)malloc(1 * LINE_SIZE);
    c_process.number_of_trys = 1;
    return c_process;
}

void copy_process(int all_childs, ProcessConfigurations *conf, ChildInfo **childs, Command *com)
{
    conf->process_created_inconfiguration = (ChildInfo **)malloc(all_childs * sizeof(ChildInfo *));
    if (conf->process_created_inconfiguration == NULL)
        ERROR_HANDLER_AND_DIE("cant allocate!");
    int i = 0;
    double exec_time = 0.0;
    for (i = 0; i < all_childs && childs[i] != NULL; i++)
    {
        if (childs[i]->id == ENQUEU_ID || childs[i] == NULL)
            continue;

        if (childs[i]->is_parrent == IS_PARRENT)
            exec_time += childs[i]->execution_time;

        conf->process_created_inconfiguration[i] = create_new_process_ptr_info(childs[i]->id,
                                                                               childs[i]->parrent_id,
                                                                               childs[i]->is_child,
                                                                               childs[i]->is_parrent,
                                                                               childs[i]->process_number,
                                                                               childs[i]->parrent_number);
        if (childs[i]->is_parrent == IS_PARRENT)
            strcpy(conf->process_created_inconfiguration[i]->command,
                   "Parrent Level 1 Doesn't Have Commnad. \0");
        else
            strcpy(conf->process_created_inconfiguration[i]->command,
                   com->commands[((childs[i]->parrent_number - 1) + (childs[i]->process_number - 1))]);

        conf->process_created_inconfiguration[i]->end_time = childs[i]->start_time;
        conf->process_created_inconfiguration[i]->end_time = childs[i]->end_time;
        conf->process_created_inconfiguration[i]->execution_time = childs[i]->execution_time;
        conf->process_created_inconfiguration[i]->exit_status = childs[i]->exit_status;
        conf->process_created_inconfiguration[i]->number_of_trys = childs[i]->number_of_trys;
        free(childs[i]);
    }
    conf->actual_time = exec_time;
    conf->childs_size = i;
}

/* 
  mimic the queue for queue of childs
    return the allocated size of new Holder
*/
int enqueue(ChildInfo **holder, ChildInfo *c_process_ptr, ChildInfo c_process, int MAX_QUEUE_SIZE, int *counter)
{
    int count = 0;
    if (holder[*counter]->id != ENQUEU_ID)
    {
        for (int i = 0; i < MAX_QUEUE_SIZE; i++, count++)
        {
            if (holder[i]->id == ENQUEU_ID)
                break;
        }
        *counter = count;
    }

    // reallocating
    if (count > MAX_QUEUE_SIZE)
    {
        holder = (ChildInfo **)realloc(holder, (sizeof(ChildInfo *) * count));
        if (!holder)
            ERROR_HANDLER_AND_DIE("reallocating wasn't sucessfull!!!!");
        for (int i = MAX_QUEUE_SIZE; i < count; i++)
            holder[i] = NULL;
    }

    if (c_process_ptr == NULL)
    {
        holder[*counter]->id = c_process.id;
        holder[*counter]->parrent_id = c_process.parrent_id;
        holder[*counter]->is_child = c_process.is_child;
        holder[*counter]->is_parrent = c_process.is_parrent;
        holder[*counter]->process_number = c_process.process_number;
        holder[*counter]->parrent_number = c_process.parrent_number;
        // strcpy(holder[count]->command, c_process.command);
        holder[*counter]->end_time = c_process.end_time;
        holder[*counter]->execution_time = c_process.execution_time;
        holder[*counter]->exit_status = c_process.exit_status;
        holder[*counter]->number_of_trys = c_process.number_of_trys;
    }
    else
    {
        holder[*counter]->id = c_process_ptr->id;
        holder[*counter]->parrent_id = c_process_ptr->parrent_id;
        holder[*counter]->is_child = c_process_ptr->is_child;
        holder[*counter]->is_parrent = c_process_ptr->is_parrent;
        holder[*counter]->process_number = c_process_ptr->process_number;
        holder[*counter]->parrent_number = c_process_ptr->parrent_number;
        // strcpy(holder[count]->command, c_process_ptr->command);
        holder[*counter]->end_time = c_process_ptr->end_time;
        holder[*counter]->execution_time = c_process_ptr->execution_time;
        holder[*counter]->exit_status = c_process_ptr->exit_status;
        holder[*counter]->number_of_trys = c_process_ptr->number_of_trys;
    }
    (*counter) = ((*counter) + 1);
    return count > MAX_QUEUE_SIZE ? count : MAX_QUEUE_SIZE;
}

void set_end_time_and_status_for_terminated_process(
    ChildInfo **holder,
    ChildInfo holder_1[],
    pid_t pid,
    int exit_status,
    int MAX_QUEUE_SIZE,
    int number_of_try)
{
    if (holder != NULL)
    {
        for (int i = 0; i < MAX_QUEUE_SIZE; i++)
        {
            if (holder[i]->id == pid)
            {
                gettimeofday(&holder[i]->end_time, NULL);
                holder[i]->execution_time = COMPUTE_EXECUTION_TIME(holder[i]->start_time, holder[i]->end_time);
                holder[i]->exit_status = exit_status;
                if (number_of_try == 1)
                    holder[i]->number_of_trys++;
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
                gettimeofday(&holder_1[i].end_time, NULL);
                holder_1[i].execution_time = COMPUTE_EXECUTION_TIME(holder_1[i].start_time, holder_1[i].end_time);
                holder_1[i].exit_status = exit_status;
                if (number_of_try == 1)
                    holder_1[i].number_of_trys++;
                break;
            }
        }
    }
}

void handeling_wait_for_proccess(
    Command *command_st,
    int counter,
    int is_parrent,
    int fd[],
    ChildInfo childs_reader[],
    ChildInfo **holder,
    int *all_childs_count,
    pid_t children_id[],
    ChildInfo **childs,
    int *count)
{
    for (int i = 0; i < counter; i++)
    {
        memset(childs_reader, 0, sizeof(ChildInfo) * command_st->child_proccess_count);
        int status;
        waitpid(children_id[i], &status, 0);
        set_end_time_and_status_for_terminated_process(
            holder, NULL, children_id[i], status, command_st->commands_count, 0);
    }

    for (int i = 0; i < command_st->proccess_count; i++)
    {
        ssize_t readed_bytes_size = read(fd[2 * i + READ_END],
                                         childs_reader,
                                         sizeof(ChildInfo) * command_st->child_proccess_count);
        if (readed_bytes_size > 0)
            for (int j = 0; j < command_st->child_proccess_count; j++)
                *all_childs_count = enqueue(childs, NULL, childs_reader[j], *all_childs_count, count);
    }
}