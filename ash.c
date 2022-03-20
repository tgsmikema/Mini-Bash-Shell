#include <stdlib.h>
#include <stdio.h>
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

/*unused functions/helpers:


*/

char* read_command_line_from_input(void){

    char *command = NULL;
    ssize_t buffsize = 0;

    /* some of below code in this function referenced from https://brennan.io/2015/01/16/write-a-shell-in-c/
    TOTAL REFERENCE: (6) LINES */

    //getline() allocate dynamic memory, kind of like malloc,
    //functions return -1 on failure to read a line (including end-of-file condition).
    if (getline(&command,&buffsize,stdin) == -1){
        //feof() This function returns a non-zero value when End-of-File indicator associated 
        //with the stream is set, else zero is returned.
        if(feof(stdin)){
            exit(EXIT_SUCCESS);
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }

    // get rid of new line character parsed into the string.
    if ((command)[strlen(command) - 1] == '\n') 
    {
      ((command)[strlen(command) - 1] = '\0');
    }

    return command;
}

char** split_string_into_tokens(char* input_string){

    //printf("%s",input_string);
    const char delim[2] = " ";
    char* token;
    
    // calculate the how many space seperated words in the input string.
    int word_count = 1;
    for(int i=0;i<strlen(input_string);i++){
        if (input_string[i] == ' '){
            word_count++;
        }
    }

    // allocate enough memory for string array.
    // (+1) make space for NULL at the end of token. 
    char **tokens = (char **) malloc((word_count+1) * sizeof(char*));

    // error checking for tokens.
    if (tokens == NULL){
        perror("memory allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(input_string, delim);

    int i = 0;
    while(token != NULL) {
        tokens[i] = token;
        i++;  
        token = strtok(NULL, delim);
   }

    tokens[i] = NULL;
    return tokens;

}

int execute_cd_command(char** tokens, char* home_directory_path){
    
    if (tokens[1] == NULL){
        // if no parameter specified, change to home address.
        return(chdir(home_directory_path));
    }
    return(chdir(tokens[1]));
}

int execute_command_line(char** tokens){

    pid_t pid;
    int status_code;

    pid = fork();
    if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0){
        //child process
        return(execvp(tokens[0],tokens));
    } else {
        wait(NULL);
    }

}

int main(int argc, char* argv[]){

    //this gets the current directory - home directory as program runs according to assignment. 
    char home_directory[DIR_MAX_SIZE];
    if (getcwd(home_directory,DIR_MAX_SIZE) == NULL){
        perror("Home Directory Retrival Error");
    }

    while(1){


    printf("ash> ");


    // isatty() returns 1 if input is from stdin, or 0 if input is from file
    // if from stdin, then call read_command_line_from_input() function,
    // otherwise if from file, then call read_command_line_from_file() function.
    if (isatty(STDIN_FILENO) == 1){
        char *line = read_command_line_from_input();
    
        char **tokens = split_string_into_tokens(line);

        // checking if command is cd, if yes carry out cd command otherwise normal commands
        if (strcmp(tokens[0],"cd") == 0){
            if ((execute_cd_command(tokens,home_directory)) == -1){
                perror("Directory Error");
            }
        } else if (execute_command_line(tokens) == -1){
           perror("Execution Error");
        };
      
    // if open commands from file:
    } else {

    }
  
    }
    
    return 1;
}