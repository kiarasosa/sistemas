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

    unsigned int length = scommand_length(cmd); // Obtiene la cantidad de argumentos en el comando
    char **arg = malloc(sizeof(char*) * (length + 1)); // Asigna memoria para los argumentos, incluyendo NULL al final (+1)
    for (unsigned int i = 0; i < length; i++){
        if(!scommand_is_empty(cmd)){
            arg[i] = malloc(sizeof(char) * (strlen(scommand_front(cmd)) + 1)); // Asigna memoria para cada argumento
            strcpy(arg[i], scommand_front(cmd)); // Copia el argumento actual al arreglo arg
            scommand_pop_front(cmd); // Elimina el argumento ya copiado del scommand
        }
    }
    
    char *file_in = scommand_get_redir_in(cmd); // Obtiene el archivo de redirección de entrada, si existe
    if(file_in != NULL){
        int redir_in = open(file_in, O_RDONLY, 0); // Abre el archivo para lectura
        if (redir_in == -1) {
            perror("open redir_in"); 
            exit(EXIT_FAILURE); 
        }
        if (dup2(redir_in, STDIN_FILENO) == -1) { // Duplica el descriptor de archivo a STDIN
            perror("dup2 redir_in"); 
            exit(EXIT_FAILURE); 
        }
        close(redir_in);  después de duplicar
    }
    
    char *file_out = scommand_get_redir_out(cmd); // Obtiene el archivo de redirección de salida, si existe
    if(file_out != NULL){
        int redir_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); // Abre/crea el archivo para escritura
        if (redir_out == -1) {
            perror("open redir_out"); 
            exit(EXIT_FAILURE); 
        }
        if (dup2(redir_out, STDOUT_FILENO) == -1) { // Duplica el descriptor de archivo a STDOUT
            perror("dup2 redir_out"); 
            exit(EXIT_FAILURE); 
        }
        close(redir_out);  después de duplicar
    }

    arg[length] = NULL; 

    execvp(arg[0], arg); // Ejecuta el comando usando `execvp`
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
        pipeline_pop_front(apipe); /
        if (pipeline_is_empty(apipe)) { 
            return;
        }
    }

    if (builtin_alone(apipe)) { 
        builtin_run(pipeline_front(apipe)); 
        return;
    }

    if (!pipeline_get_wait(apipe)){ // Si el pipeline debe ejecutarse en background
        signal(SIGCHLD, SIG_IGN); // Ignora la señal SIGCHLD para evitar zombies (procesos hijos que han terminado su ejecución pero que aún no han sido limpiados por su proceso padre)
    }

    unsigned int length = pipeline_length(apipe); 
    int *pidhijo = malloc(sizeof(int) * length);
    int pipefd[2] = {-1, -1}; // Inicializa los descriptores de la tubería actual
    int pipefd_tmp[2]; // Descriptores de la tubería anterior
    int status; // Variable para almacenar el estado de los hijos
    pid_t pid1; // Variable para almacenar el PID del proceso hijo

    pid_t *children_pids = malloc(sizeof(pid_t) * length); // Arreglo para almacenar los PIDs de los hijos

    for (unsigned int i = 0; i < length; i++) { 
        if (i != 0) { 
            pipefd_tmp[0] = pipefd[0]; // Guarda los descriptores de la tubería anterior
            pipefd_tmp[1] = pipefd[1];
        }

        if (i != length - 1) { // Si no es el último comando
            if (pipe(pipefd) == -1) { // Crea una nueva tubería
                perror("ERROR OPENING THE PIPE"); // Imprime error si falla
                exit(EXIT_FAILURE); 
            }
        }

        pid1 = fork(); // Crea un proceso hijo

        if (pid1 == -1) { // Si `fork` falla
            perror("ERROR EXECUTING FORK"); 
            exit(EXIT_FAILURE); 
        } else if (pid1 == 0) { // En el proceso hijo
            if (i != 0) { 
                close(pipefd_tmp[1]); // Cierra el descriptor de escritura de la tubería anterior
                if (dup2(pipefd_tmp[0], STDIN_FILENO) == -1) { // Duplica el descriptor de lectura a STDIN
                    perror("dup2 pipefd_tmp[0]"); 
                    exit(EXIT_FAILURE); 
                }
                close(pipefd_tmp[0]); 
            }

            if (i != length - 1) { 
                close(pipefd[0]); /
                if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                    perror("dup2 pipefd[1]"); 
                    exit(EXIT_FAILURE); 
                }
                close(pipefd[1]); 
            }

            char *command_str = scommand_to_string(pipeline_front(apipe)); // Convierte el comando a cadena para mensajes de error
            execute_scommand(pipeline_front(apipe)); // Ejecuta el comando
            fprintf(stderr, "Error executing: %s\n", command_str);
            free(command_str); 
            exit(EXIT_FAILURE); 
        } else { // proceso padre
            if (i != 0) { 
                close(pipefd_tmp[0]); // Cierra los descriptores de la tubería anterior
                close(pipefd_tmp[1]);
            }
            children_pids[i] = pid1; // Almacena el PID del hijo
            pipeline_pop_front(apipe); // Elimina el comando ya ejecutado del pipeline
        }
    }

    if (pipeline_get_wait(apipe)) { // Si se debe esperar a que los hijos terminen
        for (unsigned int i = 0; i < length; i++) { // Itera sobre todos los PIDs
            waitpid(children_pids[i], &status, 0); // Espera a que cada hijo termine
        }
    }

    free(pidhijo); 
    free(children_pids); // Libera el arreglo de PIDs
}



/* Verificaciones Iniciales:

Asegura que el pipeline no sea NULL.
Si el pipeline está vacío, termina la función.
Si el primer comando en el pipeline está vacío, lo elimina y verifica nuevamente si el pipeline está vacío.
Ejecución de Comandos Internos:

Verifica si el pipeline contiene únicamente un comando interno (builtin).
Si es así, ejecuta el comando interno utilizando builtin_run y termina la función.
Configuración para Ejecución en Background:

Si el pipeline debe ejecutarse en background (wait = false), ignora la señal SIGCHLD para prevenir procesos zombies.
Preparación para la Ejecución del Pipeline:

Obtiene la cantidad de comandos en el pipeline.
Asigna memoria para almacenar los PIDs de los procesos hijos.
Inicializa los descriptores de las tuberías.
Iteración y Ejecución de Cada Comando:

Itera sobre cada comando en el pipeline.
Para cada comando, realiza lo siguiente:
Configuración de Tuberías:
Si no es el primer comando, reutiliza los descriptores de la tubería anterior.
Si no es el último comando, crea una nueva tubería para conectar con el siguiente comando.
Creación de Proceso Hijo:
Llama a fork() para crear un proceso hijo.
En el Proceso Hijo:
Si no es el primer comando, redirige la entrada estándar al descriptor de lectura de la tubería anterior.
Si no es el último comando, redirige la salida estándar al descriptor de escritura de la tubería actual.
Convierte el comando a cadena para posibles mensajes de error.
Ejecuta el comando usando execute_scommand.
Si execute_scommand falla, imprime un mensaje de error y termina el proceso hijo.
En el Proceso Padre:
Si no es el primer comando, cierra los descriptores de la tubería anterior.
Almacena el PID del hijo en el arreglo children_pids.
Elimina el comando ya ejecutado del pipeline.
Esperar a que los Procesos Hijos Terminen:

Si el pipeline debe ejecutarse en primer plano (wait = true), espera a que todos los procesos hijos terminen utilizando waitpid.
Limpieza Final:

Libera la memoria asignada para almacenar los PIDs de los hijos. */
