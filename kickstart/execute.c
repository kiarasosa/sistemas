#include "execute.h"
#include "command.h"
#include "builtin.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "tests/syscall_mock.h"

void execute_scommand(scommand cmd){
    assert(cmd != NULL && !scommand_is_empty(cmd));

    unsigned int length = scommand_length(cmd); 
    char **arg = malloc(sizeof(char*) * (length + 1)); 
    for (unsigned int i = 0; i < length; i++){
        if(!scommand_is_empty(cmd)){
            arg[i] = malloc(sizeof(char) * (strlen(scommand_front(cmd)) + 1)); 
            strcpy(arg[i], scommand_front(cmd)); 
            scommand_pop_front(cmd); 
        }
    }
    
    char *file_in = scommand_get_redir_in(cmd);
    if(file_in != NULL){
        int redir_in = open(file_in, O_RDONLY, 0); 
        if (redir_in == -1) {
            perror("open redir_in");
            exit(EXIT_FAILURE);
        }
        if (dup2(redir_in, STDIN_FILENO) == -1) {
            perror("dup2 redir_in");
            exit(EXIT_FAILURE);
        }
        close(redir_in); 
    }
    
    char *file_out = scommand_get_redir_out(cmd);
    if(file_out != NULL){
        int redir_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (redir_out == -1) {
            perror("open redir_out");
            exit(EXIT_FAILURE);
        }
        if (dup2(redir_out, STDOUT_FILENO) == -1) {
            perror("dup2 redir_out");
            exit(EXIT_FAILURE);
        }
        close(redir_out); 
    }

    arg[length] = NULL;

    execvp(arg[0], arg); 
    perror("execvp");

    for(unsigned int i = 0; i < length; i++){
        free(arg[i]); 
    }
    free(arg);

}

void execute_pipeline(pipeline apipe) {
    assert(apipe != NULL);

    if (pipeline_is_empty(apipe)) {
        return;
    }

    if (scommand_is_empty(pipeline_front(apipe))) {
        pipeline_pop_front(apipe);
        if (pipeline_is_empty(apipe)) {
            return;
        }
    }

    if (builtin_alone(apipe)) {
        builtin_run(pipeline_front(apipe));
        return;
    }

    if (!pipeline_get_wait(apipe)){  
    signal(SIGCHLD, SIG_IGN);
    }


    unsigned int length = pipeline_length(apipe);
    int *pidhijo = malloc(sizeof(int) * length);
    int pipefd[2] = {-1, -1};
    int pipefd_tmp[2];
    int status;
    pid_t pid1;

    pid_t *children_pids = malloc(sizeof(pid_t) * length);

    for (unsigned int i = 0; i < length; i++) {
        if (i != 0) {
            pipefd_tmp[0] = pipefd[0];
            pipefd_tmp[1] = pipefd[1];
        }

        if (i != length - 1) {
            if (pipe(pipefd) == -1) {
                perror("ERROR OPENING THE PIPE");
                exit(EXIT_FAILURE);
            }
        }

        pid1 = fork(); // creamos un proceso hijo

        if (pid1 == -1) {
            perror("ERROR EXECUTING FORK");
            exit(EXIT_FAILURE);
        } else if (pid1 == 0) {
            if (i != 0) { // Redirigir la entrada si no es el primer comando
                close(pipefd_tmp[1]);
                if (dup2(pipefd_tmp[0], STDIN_FILENO) == -1) {
                    perror("dup2 pipefd_tmp[0]");
                    exit(EXIT_FAILURE);
                }
                close(pipefd_tmp[0]);
            }

            if (i != length - 1) { // Redirigir la salida si no es el último comando
                close(pipefd[0]);
                if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                    perror("dup2 pipefd[1]");
                    exit(EXIT_FAILURE);
                }
                close(pipefd[1]);
            }

            char *command_str = scommand_to_string(pipeline_front(apipe));
            execute_scommand(pipeline_front(apipe));
            fprintf(stderr, "Error executing: %s\n", command_str);
            free(command_str);
            exit(EXIT_FAILURE);
        } else {
            if (i != 0) {
                close(pipefd_tmp[0]);
                close(pipefd_tmp[1]);
            }
            children_pids[i] = pid1;
            pipeline_pop_front(apipe);
        }
    }

    if (pipeline_get_wait(apipe)) {
        for (unsigned int i = 0; i < length; i++) {
            waitpid(children_pids[i], &status, 0);
        }
    }

    free(pidhijo);
    free(children_pids);
}