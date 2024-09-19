#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>
#include <string.h>

#include "command.h"
#include "strextra.h"

typedef struct scommand_s * scommand;


struct scommand_s {
    GQueue* cmd;          
    char *redir_out;      
    char *redir_in;       
};

/* Invariantes */

static bool scommand_inv(scommand self){
    return self != NULL;
}

static bool pipeline_inv(pipeline self){
    return self != NULL;
}

// Crea e inicializa un nuevo `scommand`
scommand scommand_new(void){
    // Asigna memoria para la nueva estructura `scommand_s`
    scommand scmd_new = malloc(sizeof(struct scommand_s));
    if(scmd_new == NULL){
        printf("Error\n");
        exit(EXIT_FAILURE); // Sale si la asignación falla
    }

    scmd_new->cmd = g_queue_new(); // Inicializa la cola de comandos
    scmd_new->redir_in = NULL;     // Inicializa redirección de entrada a NULL
    scmd_new->redir_out = NULL;    // Inicializa redirección de salida a NULL 

    assert(scmd_new != NULL && scommand_is_empty(scmd_new) &&
           scommand_get_redir_in(scmd_new) == NULL &&
           scommand_get_redir_out(scmd_new) == NULL);

    return scmd_new; 
}

// Destruye y libera la memoria de un `scommand`
scommand scommand_destroy(scommand self){
    assert(scommand_inv(self));

    if(self->cmd != NULL){
       g_queue_free_full(self->cmd, free); // Libera la cola y sus elementos
       self->cmd = NULL; 
    }
    
    free(self->redir_in); 
    self->redir_in = NULL;

    free(self->redir_out); 
    self->redir_out = NULL;

    free(self); 
    return self; 
}

/* Modificadores */

// Agrega un argumento al final de la cola de comandos
void scommand_push_back(scommand self, char * argument){
    assert(scommand_inv(self) && argument != NULL);

    if(self->cmd == NULL){
        self->cmd = g_queue_new(); // Inicializa la cola si es NULL
    }
    g_queue_push_tail(self->cmd, argument); // Agrega el argumento al final

    assert(!scommand_is_empty(self));
}

// Elimina el primer argumento de la cola de comandos
void scommand_pop_front(scommand self){
    assert(scommand_inv(self) && !scommand_is_empty(self));

    free(g_queue_pop_head(self->cmd)); // Libera y elimina el primer elemento
}

// Establece el archivo de redirección de entrada
void scommand_set_redir_in(scommand self, char * filename){
    assert(scommand_inv(self));

    free(self->redir_in); // Libera cualquier redirección previa
    self->redir_in = NULL;
    self->redir_in = filename; // Asigna la nueva redirección
}

// Establece el archivo de redirección de salida
void scommand_set_redir_out(scommand self, char * filename){
    assert(scommand_inv(self));

    free(self->redir_out); // Libera cualquier redirección previa
    self->redir_out = NULL;
    self->redir_out = filename; // Asigna la nueva redirección
}

/* Proyectores */

// Verifica si la cola de comandos está vacía
bool scommand_is_empty(const scommand self){
    assert(scommand_inv(self));

    if(self->cmd == NULL){
        return true;    // Si la cola es NULL, está vacía
    }

    bool scmd_is_empty = (g_queue_get_length(self->cmd) == 0);
    
    return scmd_is_empty;
}

// Retorna la longitud de la cola de comandos
unsigned int scommand_length(const scommand self){
    assert(scommand_inv(self));

    int scmd_length = g_queue_get_length(self->cmd); 

    return scmd_length;
}

// Obtiene el primer argumento de la cola sin eliminarlo
char * scommand_front(const scommand self){ y que la cola no esté vacía
    assert(scommand_inv(self) && !scommand_is_empty(self));

    char * scmd_front = g_queue_peek_head(self->cmd); // Obtiene el primer elemento

    return scmd_front;
}

// Obtiene el archivo de redirección de entrada
char * scommand_get_redir_in(const scommand self){
    assert(scommand_inv(self));

    return self->redir_in;
}

// Obtiene el archivo de redirección de salida
char * scommand_get_redir_out(const scommand self){
    assert(scommand_inv(self));
    
    return self->redir_out;
}

// Convierte el `scommand` a una cadena de caracteres
char * scommand_to_string(const scommand self){
    assert(scommand_inv(self));
    
    GQueue* scmd_before_string = g_queue_copy(self->cmd); // Copia la cola de comandos
    
    char * scmd_to_string = NULL; // Inicializa la cadena resultante a NULL
    
    while (!g_queue_is_empty(scmd_before_string)){
        char * copy_head = g_queue_pop_head(scmd_before_string); // Obtiene y elimina el primer elemento
        if(scmd_to_string == NULL){
            scmd_to_string = strdup(copy_head); // Si es el primero, lo duplica directamente
        } else {
            char * space = strmerge(scmd_to_string, " "); 
            free(scmd_to_string); // Libera la cadena anterior
            scmd_to_string = strmerge(space, copy_head); // Agrega el siguiente argumento
            free(space); // Libera la cadena intermedia
        }      
    }
    // Agrega redirección de salida si existe
    if (self->redir_out != NULL){
        char * space = strmerge(scmd_to_string, "  >  "); 
        free(scmd_to_string);
        scmd_to_string = strmerge(space, self->redir_out); // Agrega el archivo de salida
        free(space);
    } 
    // Agrega redirección de entrada si existe
    if (self->redir_in != NULL){
        char * space = strmerge(scmd_to_string, "  <  "); 
        free(scmd_to_string);
        scmd_to_string = strmerge(space, self->redir_in); // Agrega el archivo de entrada
        free(space);
    } 
    // Si no hay comandos, asigna una cadena vacía
    if (scmd_to_string == NULL){
        scmd_to_string = strdup ("");
    }
    // Elimina un espacio final si existe
    else if(strlen(scmd_to_string) > 0 && scmd_to_string[strlen(scmd_to_string) - 1] == ' '){
        scmd_to_string[strlen(scmd_to_string) -1] = '\0';
    }
    g_queue_free(scmd_before_string); // Libera la cola copiada
    return scmd_to_string; 
}

typedef struct pipeline_s * pipeline;

struct pipeline_s {
    GQueue* pipe;    
    bool wait;       
};

pipeline pipeline_new(void){
    pipeline pipeline_new = malloc(sizeof(struct pipeline_s)); 
    if(pipeline_new == NULL){
        printf("Error\n");
        exit(EXIT_FAILURE); 
    }

    pipeline_new->pipe = g_queue_new(); // Inicializa la cola de comandos en el pipeline
    pipeline_new->wait = true;          // Por defecto, espera a que termine

    assert(pipeline_inv(pipeline_new) && pipeline_is_empty(pipeline_new) && pipeline_get_wait(pipeline_new));

    return pipeline_new; 
}

// Destruye y libera la memoria de un pipeline
pipeline pipeline_destroy(pipeline self){
    assert(pipeline_inv(self));

    if(self->pipe != NULL){
        unsigned int pipeline_length  = g_queue_get_length(self->pipe); 
        for(unsigned int pos = 0u; pos < pipeline_length; pos ++){
            scommand_destroy(g_queue_pop_tail(self->pipe)); // Destruye cada `scommand` en la cola
        }
        g_queue_free(self->pipe); 
    }

    free(self); 
    self = NULL;
    return self; 
}


/* Modificadores */

// Agrega un `scommand` al final del pipeline
void pipeline_push_back(pipeline self, scommand sc){
    assert(pipeline_inv(self) && sc != NULL);

    if(self->pipe == NULL){
        self->pipe = g_queue_new(); 
        g_queue_push_tail(self->pipe, sc); 
    } else {
        g_queue_push_tail(self->pipe, sc);  
    }

    assert(!pipeline_is_empty(self));
}

// Elimina el primer `scommand` del pipeline
void pipeline_pop_front(pipeline self){ y que el pipeline no esté vacío
    assert(pipeline_inv(self) && !pipeline_is_empty(self));

    scommand kill = g_queue_pop_head(self->pipe); // Obtiene y elimina el primer `scommand`
    scommand_destroy(kill); 
    kill = NULL;
}

// flag de espera (wait) del pipeline
void pipeline_set_wait(pipeline self, const bool w){
    assert(pipeline_inv(self));
    
    self->wait = w; // Asigna el valor de `wait`
}


/* Proyectores */

// Verifica si el pipeline está vacío
bool pipeline_is_empty(const pipeline self){
    assert(pipeline_inv(self));

    bool pipe_is_empty = g_queue_is_empty(self->pipe); 
    
    return pipe_is_empty;
}


// Retorna la longitud del pipeline
unsigned int pipeline_length(const pipeline self){
    assert(pipeline_inv(self));
    
    unsigned int pipe_length = g_queue_get_length(self->pipe);

    return pipe_length;
}

// Obtiene el primer `scommand` del pipeline sin eliminarlo
scommand pipeline_front(const pipeline self){ y que el pipeline no esté vacío
    assert(pipeline_inv(self) && !pipeline_is_empty(self));

    scommand pipe_front = g_queue_peek_head(self->pipe); 

    return pipe_front;
}


bool pipeline_get_wait(const pipeline self){
    assert(pipeline_inv(self));

    return self->wait;
}

// Convierte el `pipeline` a una cadena de caracteres
char * pipeline_to_string(const pipeline self){
    assert(pipeline_inv(self));

    char * result = strdup(""); // Inicializa la cadena resultante como vacía
    char * temp;
    unsigned int length = pipeline_length(self); // Obtiene la longitud del pipeline

    for (size_t i = 0; i < length; i++) {
        scommand sc = g_queue_peek_nth(self->pipe, i); // Obtiene el scommand
        char *scommand_string = scommand_to_string(sc); // Convierte scommand a cadena

        temp = strmerge(result , scommand_string); // Combina la cadena actual con el `scommand`
        free(result); // Libera la cadena anterior
        result = temp; // Asigna la nueva cadena

        if (i < length - 1){
            temp = strmerge(result, " | "); // Agrega pipe
            free(result); // Libera la cadena anterior
            result = temp; // Asigna la nueva cadena
        }
        free(scommand_string); // Libera la cadena del `scommand`
    }

    
    if (!self->wait){
        temp = strmerge(result , " &");  // & -> q el comando debe ejecutarse en segundo plano
        free(result);
        result = temp;
    }

    assert(pipeline_is_empty(self) || pipeline_get_wait(self) || strlen(result) > 0);

    return result; 
}
