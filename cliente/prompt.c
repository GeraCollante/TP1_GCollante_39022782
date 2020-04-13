#include "prompt.h"

/**
 * @brief   read line from stdin
 * @return  char * with the line
 */
char *read_line(void){
    unsigned long bufsize = LSH_RL_BUFSIZE;
    unsigned long position = 0;
    char * buffer = (char*) malloc(sizeof(char)*bufsize);
    int c;

    if (!buffer){
        perror("lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        /*  Read a character */
        c = getchar();

        /*  If we hit EOF, replace it with a null character and return. */
        if (c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        } 
        else{
            buffer[position] = (char)c;
        }
        position++;

        /*  If we have exceeded the buffer, reallocate. */
        if (position >= bufsize){
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/**
 * @brief   
 * @param   line 
 * @return  char** 
 */
char ** split_line(char *line){
    unsigned long bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL){
        tokens[position] = token;
        position++;

        if(position >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * @brief   Count the number of strings in a string array
 * @param   n array of strings
 * @return  number of strings
 */
int argc(char ** n){
    int counter = 0;
    while (*n != NULL){
        (void) *n++;
        counter++;
    }
    return counter;
}

/**
 * @brief   Select type of command
 * 
 *          In case the first argument is any of the valid commands, 
 *          the function will return an int greater than or equal to zero. 
 *          If not it will return a negative int.
 * @param   args  Array of tokenized strings coming from user prompt
 * @param   n_args Number of elements of array args
 * @return  int 
 */
int get_cmd(char ** args, int n_args){
    int status=-1;
    /*  If there is only one argument and exit was entered,
        status is set to 0  */
    if(n_args==1&&!strcmp(args[0],"exit")){
        status=0;
    }
    /*  If there is more than one argument, 
        status is set depending on the option selected  */
    else if(n_args>1){
        if(!strcmp(args[0],"user")) status = 1;
        if(!strcmp(args[0],"file")) status = 2;
    }
    /*  If not entered in any of the previous cases,
        then it is an invalid option and status remains at -1   */
    return status;
}

/**
 * @brief   Prompt for the user
 * 
 *          All possibilities are contemplated to achieve robust
 *          behavior of the function. This may make it look a
 *          bit complex but it is properly documented for understanding the code.
 * @param   str_to_server String to be sent to the server
 * @return  pointer to string @str_server
 */
char * cmd_prompt(char * str_to_server){
    char *line;
    char **args;
    int cmd, n_args, valid_cmd = 0;
    size_t min_pass = 7;
    /**
     * @brief Start of the prompt, it is kept in a do-while
     * loop until the user enters a valid command.
     */
    do{
        printf("> ");
        /* Read a line of the prompt */
        line = read_line();

        /* Tokenize by spaces */
        args = split_line(line);

        /* Get number of arguments */
        n_args = argc(args);

        /* Check if 1st argument is valid */
        cmd = get_cmd(args, n_args);
        switch (cmd){
            /* Wrong command */
            case -1:
                printf("%s\n", "Wrong command. Retry again.");
                valid_cmd = 0;
                break;
            /* Exit command */
            case 0:
                sprintf(str_to_server, "%d", cmd);
                valid_cmd = 1;
                break;
            /* Auth commands */
            case 1:
                /* user ls */
                if(!strcmp(args[1], "ls")){
                    if (n_args<3)
                    {
                        sprintf(str_to_server, "%d%d", cmd, 0);
                        printf("Correct command.\n");
                        valid_cmd = 1;
                    }
                    else{
                        printf("Incorrect number of arguments. Retry again. \n");
                    }
                }
                /* user passwd */
                if(!strcmp(args[1], "passwd")){
                    if(n_args==3&&strlen(args[2])>min_pass)
                    {    
                        sprintf(str_to_server, "%d%d,%s", cmd, 1, args[2]);
                        printf("Correct password.\n");
                        valid_cmd = 1;
                    }
                    else if (n_args==2)
                    {
                        printf("Password cannot be empty. Retry again. \n");
                    }
                    else
                    {
                        printf("Minimum password size is %ld characters. Retry again.\n", min_pass+1);
                    }
                }
                break;
            case 2:
                /* file ls */
                if(!strcmp(args[1], "ls")){
                    if (n_args<3)
                    {
                        sprintf(str_to_server, "%d%d", cmd, 0);
                        printf("Correct command. \n");
                        valid_cmd = 1;
                    }
                    else
                    {
                        printf("Incorrect number of arguments. Retry again. \n");
                    }
                }
                /* file down */
                if(!strcmp(args[1], "down")){
                    if(n_args==3)
                    {    
                        sprintf(str_to_server, "%d%d,%s", cmd, 1, args[2]);
                        printf("Correct command. \n");
                        valid_cmd = 1;
                    }
                    else
                    {
                        printf("Incorrect number of arguments. Retry again. \n");
                    }
                }
                break;
        }
        free(line);
        free(args);
    }while(!valid_cmd);

    /* User entered a valid command */
    return str_to_server;
}

/**
 * @brief   In case of the user has selected the exit command, then exit the prompt
 * @param   str str_to_server
 * @return  int 
 */
int is_exit(char * str){
    int value = (!strcmp(str,"0")) ? 0 : 1;
    return value;
}