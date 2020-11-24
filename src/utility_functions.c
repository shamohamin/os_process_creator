#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "headers.h"

int generate_divisors_of_number(int commands_count, int configuration[])
{

    int counter = 0;
    for (int i = 1; i <= commands_count; i++)
        if (commands_count % i == 0)
        {
            if (counter > commands_count)
            {
                configuration = realloc(configuration, counter);
                if (configuration == NULL)
                {
                    ERROR_HANDLER_AND_DIE("reallocation was not successfull");
                }
            }
            configuration[counter] = i;
            counter++;
        }

    return counter;
}

int parse_command_to_be_executed(char *line, char **args)
{
    int args_counting = 0;
    int start_index = 0;
    int counter = 0;
    int c = 0;

    for (counter = 0;; counter++)
    {
        if (isblank(line[counter]) || line[counter] == '\0')
        {
            c = 0;
            args[args_counting] = (char *)malloc(sizeof(char) * strlen(line));

            for (int i = start_index; i < counter; i++, c++)
                args[args_counting][c] = line[i];

            if (line[counter] == '\0')
            {
                break;
            }
            else
            {
                args[args_counting][c] = '\0';
            }
            start_index = counter + 1;
            args_counting++;
        }
    }
    args[args_counting][c] = '\0';

    for (int i = 0; i <= args_counting; i++)
    {
        if (args[i] == NULL || args[i][0] == '\0' || isblank(args[i][0]))
        {
            args_counting--;
            break;
        }
    }

    return args_counting;
}

char *generating_commands(int start_index, int last_index, Command *command_st)
{

    char *commands = (char *)malloc(LINE_SIZE * (last_index - start_index) * sizeof(char));
    int counter = 0;

    for (int i = start_index; i < last_index; i++)
    {
        int counter_commnad = 0;
        while (1)
        {
            if (
                command_st->commands[i][counter_commnad] == '\0' ||
                command_st->commands[i][counter_commnad] == 0)
                break;

            commands[counter] = command_st->commands[i][counter_commnad];
            counter++, counter_commnad++;
        }

        if (i != last_index - 1)
        {
            commands[counter - 1] = ' ';
            commands[counter] = ';';
        }

        counter++;
    }

    commands[counter] = '\0';

    return commands;
}

char *split_the_generated_commnad(char src[], int pos, int count)
{
    char **temp = (char **)malloc(sizeof(char *) * count);
    for (int i = 0; i < count; i++)
        temp[i] = (char *)malloc(sizeof(char) * LINE_SIZE * count);

    int counter = 0;
    int last_index, first_index;
    int tmp_index = 0;
    int c = 0;

    for (first_index = 0, last_index = 0;; counter++)
    {
        if (src[counter] == '\0')
            break;
        if (src[counter] == ';')
        {
            last_index = counter;
            c = 0;
            for (int i = first_index; i < last_index; i++, c++)
                temp[tmp_index][c] = src[i];

            temp[tmp_index][c] = '\0';
            first_index = last_index + 1;
            tmp_index++;
        }
    }

    last_index = counter;
    c = 0;
    for (int i = first_index; i < last_index; i++, c++)
        temp[tmp_index][c] = src[i];
    temp[tmp_index][c] = '\0';

    for (int i = 0; i < tmp_index && i != pos; i++)
        free(temp[i]);

    return temp[pos];
}

void create_childs_process()
{
}

/*
    this function is used for removing the spaces between the args and commands;
*/
char *line_convertion(char *line)
{
    char *temp = (char *)malloc(sizeof(char) * 100);
    int i = 0;
    int counter = 0;
    int flag = 1;

    while ((line[i] != '\0' || line[i] != 0))
    {
        if (!isblank(line[i]))
        {
            temp[counter] = line[i];
            counter++;
            flag = 1;
        }

        if (isblank(line[i]) && flag)
        {
            temp[counter] = ' ';
            counter++;
            flag = 0;
        }

        i++;
    }
    temp[counter++] = '\0';

    return temp;
}
/*
    this function is used for seperating the arguments of command and time for that specific command
        seperate tha line with using the args;
*/
char *seperating_values(char *line, int pos)
{
    char *value_holder[SIZE];
    int counter = 0;
    int str_counter = 0;
    int flag = 1;

    for (int i = 0; line[i] != '\0' && line[i] != 0; i++)
    {
        if (flag)
        {
            value_holder[counter] = (char *)malloc(sizeof(char) * LINE_SIZE);
            flag = 0;
        }

        if (isblank(line[i]))
        {
            value_holder[counter][str_counter] = '\0';
            counter++;
            str_counter = 0;
            flag = 1;
            continue;
        }

        value_holder[counter][str_counter] = line[i];
        str_counter++;
    }

    value_holder[counter][str_counter] = '\0';

    char *val = value_holder[pos];

    for (int i = 0; i <= counter && i != pos; i++)
        free(value_holder[i]);

    return val;
}

void reading_file(char *filepath, Command *command_st)
{
    char *buffer = (char *)malloc(sizeof(char) * LINE_SIZE);
    size_t input_size = LINE_SIZE;
    int counter = 0;

    if (buffer == NULL)
        ERROR_HANDLER_AND_DIE("Allocating wasn't successfull!!");

    FILE *f;
    if ((f = fopen(filepath, "r")) == NULL)
        ERROR_HANDLER_AND_DIE("Couldnt open the file.");

    int i = 0;
    while (fgets(buffer, LINE_SIZE, f))
    {
        char *line = buffer;
        line = line_convertion(line);

        if (i == 0)
        {
            command_st->proccess_count = (int)strtol(seperating_values(line, 0), NULL, 10);
            command_st->child_proccess_count = (int)strtol(seperating_values(line, 1), NULL, 10);
        }
        else
        {
            command_st->commands[i - 1] = (char *)malloc(strlen(line) * sizeof(char));
            strcpy(command_st->commands[i - 1], line);
        }

        // freeing readed line from file
        free(line);
        i++;
    }

    command_st->commands_count = i - 1;

    // freeing the pointers
    free(buffer);
    // closing  the file
    fclose(f);
}

// showing the process INFO
void printing_process_info(ChildInfo *c_process, Command *com)
{

    printf("***********************\n");

    if (c_process->exit_status == OK_EXIT_STATUS)
        green();
    else
        red();

    printf("id: %d\n", c_process->id);
    printf("number of process: %d\n", c_process->process_number);
    printf("number of parrent: %d\n", c_process->parrent_number);
    printf("commnad of this process: %s", c_process->command);
    printf("is_parrent: %d\n", c_process->is_parrent);
    printf("is_child: %d\n", c_process->is_child);
    printf("elapsed time: %f sec\n", c_process->execution_time);
    printf("parrent process id: %d\n", c_process->parrent_id);
    printf("exit status: %d\n", c_process->exit_status);
    printf("number_of_try: %d\n", c_process->number_of_trys);
    reset();
    printf("***********************\n");
}

// colors for console
void red()
{
    if (PLATFORM == 1)
        printf("\x1b[1;31m");
    else if (PLATFORM == 0)
        printf("\033[1;31m");
}

void green()
{
    if (PLATFORM == 1)
        printf("\x1b[0;32m");
    else if (PLATFORM == 0)
        printf("\033[0;32m");
}

void reset()
{
    if (PLATFORM == 1)
        printf("\x1b[0m");
    else if (PLATFORM == 0)
        printf("\033[0m");
}