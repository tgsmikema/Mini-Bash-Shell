#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*  Author: Siqi(Mike) Ma
    ID:     2623324
    upi:    sma148
    SOFTENG 370 Assignment 1 */

/*unused functions/helpers:
-- this gets the current directory {char *buff = NULL; printf("%s",getcwd(buff,0));}

*/

char* read_command_line_from_input(void){

    char *command = NULL;
    size_t buffsize = 0;

    /* some of below code referenced from https://brennan.io/2015/01/16/write-a-shell-in-c/
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

    return command;
}

int main(int argc, char* argv[]){

    while(1){

    printf("ash> ");


    // isatty() returns 1 if input is from stdin, or 0 if input is from file
    // if from stdin, then call read_command_line_from_input() function,
    // otherwise if from file, then call read_command_line_from_file() function.
    if (isatty(STDIN_FILENO) == 1){
        char *line = read_command_line_from_input();
        // developer test line:
        printf("%s",line);
    } else {

    }
  
    }
    
    return 0;
}