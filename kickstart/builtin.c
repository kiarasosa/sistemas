#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include "command.h"
#include "builtin.h"
#include "execute.h"
#include "tests/syscall_mock.h"


bool builtin_is_internal(scommand cmd){
    assert(cmd != NULL && !scommand_is_empty(cmd));

    const char *internal_cmd[] = {"cd", "help", "exit"};
    const unsigned int cant_cmd = sizeof(internal_cmd)/sizeof(internal_cmd[0]);
    char *cmd_front = scommand_front(cmd);
    for(unsigned int i = 0; i < cant_cmd; i++){
        if(strcmp(cmd_front, internal_cmd[i]) == 0){
            return true;
        }
    }
    return false;
}

bool builtin_alone(pipeline p){
    assert(p != NULL);

    return pipeline_length(p) == 1 && builtin_is_internal(pipeline_front(p));
}

void builtin_cd(scommand cmd) {
    assert(!scommand_is_empty(cmd));
    
    scommand_pop_front(cmd);
    char *path = NULL;

    if(scommand_is_empty(cmd)){
        path = getenv("HOME");

        if (path == NULL){
            perror("cd");
        }
    } else {
        path = scommand_front(cmd);
    }
    
    if (chdir(path) == -1){
        perror("cd");
    }
}

void builtin_help() {
    const char *help = 
        "Mybash by 'ADB'\n"
        "Authors: Salman Mauro, Sosa Kiara, Flores Lionel\n"
        "Internal commands:\n"
        "- cd: chdir() Changes the current directory\n"
        "- help: Shows this help message (Mybash information)\n"
        "- exit: exit() Exits the shell\n"
        "\n";
    printf("%s", help);
}

void builtin_exit() {
    exit(0);
}

void builtin_run(scommand cmd) {
    assert(builtin_is_internal(cmd));

    char *cmd_front = scommand_front(cmd);

    if(scommand_is_empty(cmd)){
        return;
    }

    if (strcmp(cmd_front, "cd") == 0) {
        builtin_cd(cmd);
    } else if(strcmp(cmd_front, "help") == 0){
        builtin_help();
    } else if (strcmp(cmd_front, "exit") == 0) {
        builtin_exit();
    }
}
