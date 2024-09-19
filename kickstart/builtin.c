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

// Función que verifica si el comando dado es interno (cd, help, exit)
bool builtin_is_internal(scommand cmd){
    assert(cmd != NULL && !scommand_is_empty(cmd));  // Verifica que el comando no sea nulo ni esté vacío

    const char *internal_cmd[] = {"cd", "help", "exit"};  // Lista de comandos internos
    const unsigned int cant_cmd = sizeof(internal_cmd)/sizeof(internal_cmd[0]);  // Cantidad de comandos internos
    char *cmd_front = scommand_front(cmd);  // Obtiene el primer elemento del comando
    for(unsigned int i = 0; i < cant_cmd; i++){  // Itera sobre la lista de comandos internos
        if(strcmp(cmd_front, internal_cmd[i]) == 0){  // Compara el primer comando con los comandos internos
            return true;  // Es un comando interno
        }
    }
    return false;  // No es un comando interno
}

// Verifica si un pipeline contiene un único comando que es interno
bool builtin_alone(pipeline p){
    assert(p != NULL); 

    return pipeline_length(p) == 1 && builtin_is_internal(pipeline_front(p));  // Retorna true si el pipeline contiene solo un comando y es interno
}

// Comando 'cd'
void builtin_cd(scommand cmd) {
    assert(!scommand_is_empty(cmd));  

    scommand_pop_front(cmd);  // Elimina el comando 'cd' de la lista
    char *path = NULL;

    if(scommand_is_empty(cmd)){  // Si no se proporciona un directorio
        path = getenv("HOME");  // Utiliza el directorio HOME por defecto

        if (path == NULL){  // Si no se encuentra el directorio HOME
            perror("cd");  
        }
    } else {
        path = scommand_front(cmd);  // Obtiene el directorio proporcionado como argumento
    }
    
    if (chdir(path) == -1){  // Cambia el directorio actual, si falla, imprime un error
        perror("cd");
    }
}

// Comando 'help'
void builtin_help() {
    const char *help =  // Mensaje de ayuda que describe los comandos internos
        "Mybash by 'ADB'\n"
        "Authors: Salman Mauro, Sosa Kiara, Flores Lionel\n"
        "Internal commands:\n"
        "- cd: chdir() Changes the current directory\n"
        "- help: Shows this help message (Mybash information)\n"
        "- exit: exit() Exits the shell\n"
        "\n";
    printf("%s", help);  // Imprime el mensaje de ayuda
}

// Comando 'exit'
void builtin_exit() {
    exit(0);  // Termina la ejecución del shell
}

// Función que ejecuta un comando interno
void builtin_run(scommand cmd) {
    assert(builtin_is_internal(cmd));  // Verifica que el comando sea interno

    char *cmd_front = scommand_front(cmd);  // Obtiene el primer elemento del comando

    if(scommand_is_empty(cmd)){  // Si el comando está vacío, simplemente retorna
        return;
    }

    if (strcmp(cmd_front, "cd") == 0) {  // Si el comando es 'cd'
        builtin_cd(cmd);  // Llama a la función para manejar 'cd'
    } else if(strcmp(cmd_front, "help") == 0){  // Si el comando es 'help'
        builtin_help();  // Llama a la función para mostrar ayuda
    } else if (strcmp(cmd_front, "exit") == 0) {  // Si el comando es 'exit'
        builtin_exit();  // Llama a la función para salir del shell
    }
}
