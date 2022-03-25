#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

/*  Author: Siqi(Mike) Ma
    ID:     2623324
    upi:    sma148
    SOFTENG 370 Assignment 1 */

#define DIR_MAX_SIZE 256
#define HISTORY_STRING_SIZE 100
#define PRINT_HISTORY_SIZE 10

// Global variables:

int wstatus;

/*unused functions/helpers:


*/
void handle_sigtstp(int sig)
{
    printf("Stop not allowed\n");
}

int size_of_star_star(char **starstar)
{
    int count = 0;
    while ((*starstar) != NULL)
    {
        count++;
        starstar++;
    }
    return count;
}

int size_of_triple_star(char ***triple_star)
{
    int count = 0;
    while ((*triple_star) != NULL)
    {
        count++;
        triple_star++;
    }
    return count;
}

char *read_cmd_line_into_string(void)
{

    char *command = NULL;
    ssize_t buffsize = 1;

    /* some of below code in this function referenced from https://brennan.io/2015/01/16/write-a-shell-in-c/
    TOTAL REFERENCE: (6) LINES */

    // getline() allocate dynamic memory, kind of like malloc,
    // functions return -1 on failure to read a line (including end-of-file condition).
    if (getline(&command, &buffsize, stdin) == -1)
    {
        // feof() This function returns a non-zero value when End-of-File indicator associated
        // with the stream is set, else zero is returned.
        if (feof(stdin))
        {
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }

    // get rid of new line character parsed into the string.
    if ((command)[strlen(command) - 1] == '\n')
    {
        ((command)[strlen(command) - 1] = '\0');
    }

    // fix empty command segmentation fault;
    if ((command)[strlen(command) - 1] == '\0')
    {
        command = strdup("aaa");
    }

    return command;
}

char **breakup_piped_string_into_simple_strings(char *complex_string)
{

    // printf("%s",input_string);
    const char delim[3] = "|";
    char *token;

    // calculate the how many | seperated commmands in the input string.
    int word_count = 1;
    for (int i = 0; i < strlen(complex_string); i++)
    {
        if (complex_string[i] == '|')
        {
            word_count++;
        }
    }

    // allocate enough memory for string array.
    // (+1) make space for NULL at the end of token.
    char **tokens = (char **)malloc((word_count + 1) * sizeof(char *));

    // error checking for tokens.
    if (tokens == NULL)
    {
        perror("memory allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(complex_string, delim);

    int i = 0;
    while (token != NULL)
    {
        tokens[i] = token;
        i++;
        token = strtok(NULL, delim);
    }

    tokens[i] = NULL;
    return tokens;
}

char **split_string_into_tokens(char *input_string)
{

    // printf("%s",input_string);
    const char delim[10] = " &\t";
    char *token;

    // calculate the how many space seperated words in the input string.
    int word_count = 1;
    for (int i = 0; i < strlen(input_string); i++)
    {
        if (input_string[i] == ' ')
        {
            word_count++;
        }
    }

    // allocate enough memory for string array.
    // (+1) make space for NULL at the end of token.
    char **tokens = (char **)malloc((word_count + 1) * sizeof(char *));

    // error checking for tokens.
    if (tokens == NULL)
    {
        perror("memory allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(input_string, delim);

    // checking if token is started with empty string, then insert arbitraty string to show user warning message.
    if (token == NULL)
    {
        token = strdup("Please Enter a VALID Input!!");
    }

    int i = 0;
    while (token != NULL)
    {
        tokens[i] = token;
        i++;
        token = strtok(NULL, delim);
    }

    tokens[i] = NULL;
    return tokens;
}

char ***convert_piped_string_into_tokens_array(char *complex_string)
{

    char **simple_strings = breakup_piped_string_into_simple_strings(complex_string);

    unsigned long size_of_simple_strings = (unsigned long)size_of_star_star(simple_strings);
    size_of_simple_strings++;

    char ***tokens_array = (char ***)malloc((size_of_simple_strings) * sizeof(char *));

    for (int i = 0; i < size_of_star_star(simple_strings); i++)
    {

        tokens_array[i] = split_string_into_tokens(simple_strings[i]);
    }

    tokens_array[size_of_star_star(simple_strings)] = NULL;

    // printf("%s\n",**(tokens_array+3));

    return tokens_array;
}

int execute_cd_command(char **tokens, char *home_directory_path)
{

    // checking cd command can take maximum 1 argument after 'cd'.
    if (tokens[2] != NULL)
    {
        return -1;
    }

    if (tokens[1] == NULL)
    {
        // if no parameter specified, change to home address.
        return (chdir(home_directory_path));
    }
    return (chdir(tokens[1]));
}

int execute_single_command(char **tokens)
{
    // put all | and & functionality in this function so it can be reused by the file input option too.

    pid_t pid;
    int status_code;

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
    }

    if (pid == 0)
    {
        // child process
        return (execvp(tokens[0], tokens));
    }
    else
    {
        wait(NULL);
    }
}

/*This function checks whether command line contains 'pipe'
returns a interger 0 if found. Otherwise returns -1 of not found.*/
int is_command_including_pipe(char *command_line_string)
{

    if (strstr(command_line_string, "|"))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/*This function checks whether command line contains 'pipe'
returns a interger 0 if found. Otherwise returns -1 of not found.*/
int is_command_including_amper(char *command_line_string)
{
    if (strstr(command_line_string, "&"))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/* This function is sourced from: https://gist.github.com/aspatic/93e197083b65678a132b9ecee53cfe86*/
int pipeline_execution(char ***tokens_array)
{
    int pipefd[2];

    int previous_fd = 0;

    // printf("2nd - 1: %s\n2nd - 2: %s\n", **(tokens_array+1), *(*(tokens_array+1)));

    while (*tokens_array != NULL)
    {

        pid_t pid_a;

        if (pipe(pipefd) == -1)
        {
            printf("Error opening pipe !\n");
            return -1;
        }

        if ((pid_a = fork()) == -1)
        {
            perror("fork");
            return -1;
        }

        else if (pid_a == 0)
        {
            // child process.
            //  replace stdin to previous fdd. (or 0 (read) first iteration)
            dup2(previous_fd, STDIN_FILENO);
            // if there are more commands
            if (*(tokens_array + 1) != NULL)
            {
                // then replace stdout with write end of pipe
                dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[0]);
            // close(pipefd[0]);
            close(pipefd[1]);
            execvp((*tokens_array)[0], *tokens_array);
            // below line will never excute due to execvp hijack the current process within the child process.

            return -1;
        }
        else
        {
            waitpid(pid_a, &wstatus, WUNTRACED);
            close(pipefd[1]);
            previous_fd = pipefd[0];
            tokens_array++;

            // printf("%s\n",**tokens_array);
        }
    }
    execvp(**tokens_array, *tokens_array);

    return 0;
}

/*This function is refered from the status.c file shared on the tutorial*/
char get_status_of_process(pid_t me){

    char c;
    char pidtext[10];
    char procfilename[100];
    FILE *procfile;

    sprintf(pidtext, "%d", me);
    strcpy(procfilename, "/proc/");
    strcat(procfilename, pidtext);
    strcat(procfilename, "/stat");
    //printf("The status file is %s\n", procfilename);
    procfile = fopen(procfilename, "r");
    if (procfile == NULL)
    {
        perror("Failed to open file.");
        exit(EXIT_FAILURE);
    }
    do
    {
        c = fgetc(procfile);
    } while (c != ')');
    fgetc(procfile);
    c = fgetc(procfile);
    //printf("The status of this process is %c\n", c);
    fclose(procfile);

    return c;
}

int main(int argc, char *argv[])
{

    struct sigaction sa;
    sa.sa_handler = &handle_sigtstp;
    sa.sa_flags = SA_RESTART;

    char **history_list = (char **)malloc((100) * sizeof(char *));

    int last_history_position = 0;

    //---------------------------------------------------------------------------------------------
    // this gets the current directory - home directory as program runs according to assignment.
    char home_directory[DIR_MAX_SIZE];
    if (getcwd(home_directory, DIR_MAX_SIZE) == NULL)
    {
        perror("Home Directory Retrival Error");
    }
    //---------------------------------------------------------------------------------------------

    while (1)
    {

        printf("ash> ");

        sigaction(SIGTSTP, &sa, NULL);

        // isatty() returns 1 if input is from stdin, or 0 if input is from file
        // if from stdin, then call read_command_line_from_input() function,
        // otherwise if from file, then call read_command_line_from_file() function.

        char *line = read_cmd_line_into_string();
        int is_ampersand = is_command_including_amper(line);

        if (isatty(STDIN_FILENO) != 1)
        {
            printf("%s\n", line);
        }

        history_list[last_history_position] = strdup(line);
        last_history_position++;
        // printf("history NO!!!!!!!! %d\n",last_history_position);

        // this changed line after this function.
        char ***tokens_array = convert_piped_string_into_tokens_array(line);

        int num_of_pipe_args = size_of_triple_star(tokens_array);
        
        // checking if command is 'cd', if yes carry out cd command otherwise normal commands ------------------

        if (strcmp(**tokens_array, "cd") == 0)
        {
            if ((execute_cd_command(*tokens_array, home_directory)) == -1)
            {
                perror("Directory Error");
            }
            // checking if command is 'history'. -------------------------------------------------------------------
        }
        else if ((strcmp(**tokens_array, "history") == 0) || (strcmp(**tokens_array, "h") == 0))
        {

            // get arg size including 'history' and parameters.
            int args_size = size_of_star_star(*tokens_array);

            if (args_size > 2)
            {
                perror("history parameter error");
                continue;
            }
            else if (args_size == 1)
            {
                // print history statements
                int i = 0;
                if (last_history_position > 9)
                {
                    i = last_history_position - 10;
                }
                for (i; i < last_history_position; i++)
                {
                    printf("     %d: %s\n", i + 1, history_list[i]);
                }
            }
            else if (args_size == 2)
            {
                int select_history_number = atoi(*(*(tokens_array) + 1));

                if ((select_history_number >= (last_history_position)) || select_history_number < 1)
                {
                    perror("History Index Out of Bound");
                    continue;
                }

                strcpy(history_list[last_history_position - 1], history_list[select_history_number - 1]);
                // last_history_position++;

                char *new_line = strdup(history_list[select_history_number - 1]);

                if (is_command_including_pipe(new_line) == -1)
                {
                    if (new_line[0] == 'h')
                    {
                        // print history statements
                        int i = 0;
                        if (last_history_position > 9)
                        {
                            i = last_history_position - 10;
                        }
                        for (i; i < last_history_position; i++)
                        {
                            printf("     %d: %s\n", i + 1, history_list[i]);
                        }
                    }
                    execute_single_command(split_string_into_tokens(new_line));
                }
                // normal command (not built in) -------------------------------------------------------------------
                else
                {
                    char ***tokens_array = convert_piped_string_into_tokens_array(new_line);
                    pid_t pid_h;

                    pid_h = fork();
                    if (pid_h == -1)
                    {
                        perror("fork");
                    }
                    if (pid_h == 0)
                    {
                        if (pipeline_execution(tokens_array) == -1)
                        {
                            perror("Command error");
                            break;
                        }
                    }
                    else
                    {
                        // only wait when there is no &.
                        if (is_ampersand == -1)
                        {
                            // printf("%d\n%d\n",pid,getppid());
                            waitpid(pid_h, &wstatus, WUNTRACED);
                            // kill(pid, SIGKILL);
                        }
                    }
                }
            }
        }
        else
        {
            pid_t pid;

            pid = fork();
            if (pid == -1)
            {
                perror("fork");
            }
            if (pid == 0)
            {

                // if single
                if (num_of_pipe_args == 1)
                {
                    execvp(*tokens_array[0], *tokens_array);
                    printf("command error: No such file or directory.\n");
                    break;
                }
                // if pipeline
                else
                {
                    int pline_status = pipeline_execution(tokens_array);

                    if (pline_status == -1)
                    {
                        perror("Command error");
                        break;
                    }
                }
            }
            else
            {
                if (is_ampersand == -1)
                {
                    // printf("status: %c\n",get_status_of_process(pid));

                    // printf("%d\n",pid);
                    waitpid(pid, &wstatus, WUNTRACED);
                    // printf("status: %d\n", wstatus);
                }
            }
        }
    }
    return 1;
}
