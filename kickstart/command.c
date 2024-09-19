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


scommand scommand_new(void){
    scommand scmd_new = malloc(sizeof(struct scommand_s));
        if(scmd_new == NULL){
            printf("Error\n");
            exit(EXIT_FAILURE);
        }

        scmd_new->cmd = g_queue_new();
        scmd_new->redir_in = NULL;
        scmd_new->redir_out = NULL; 

        assert(scmd_new != NULL && scommand_is_empty (scmd_new) && scommand_get_redir_in (scmd_new) == NULL && scommand_get_redir_out (scmd_new) == NULL);

        return scmd_new;
}

scommand scommand_destroy(scommand self){
    assert(scommand_inv(self));

    if(self->cmd != NULL){
       g_queue_free_full(self->cmd, free);
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

void scommand_push_back(scommand self, char * argument){
    assert(scommand_inv(self) && argument != NULL);

    if(self->cmd == NULL){
        self->cmd = g_queue_new();
    }
        g_queue_push_tail(self->cmd, argument);

    assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
    assert(scommand_inv(self) && !scommand_is_empty(self));

    free(g_queue_pop_head(self->cmd));
}

void scommand_set_redir_in(scommand self, char * filename){
    assert(scommand_inv(self));

    free(self->redir_in);
    self->redir_in = NULL;
    self->redir_in = filename;
}

void scommand_set_redir_out(scommand self, char * filename){
    assert(scommand_inv(self));

    free(self->redir_out);
    self->redir_out = NULL;
    self->redir_out = filename;
}

/* Proyectores */

bool scommand_is_empty(const scommand self){
    assert(scommand_inv(self));

    if(self->cmd == NULL){
        return true;    
    }

    bool scmd_is_empty = (g_queue_get_length(self->cmd) == 0);
    
    return scmd_is_empty;
}

unsigned int scommand_length(const scommand self){
    assert(scommand_inv(self));

    int scmd_length = g_queue_get_length(self->cmd);

    return scmd_length;
}

char * scommand_front(const scommand self){
    assert(scommand_inv(self) && !scommand_is_empty(self));

    char * scmd_front = g_queue_peek_head(self->cmd);

    return scmd_front;
}

char * scommand_get_redir_in(const scommand self){
    assert(scommand_inv(self));

    return self->redir_in;
}

char * scommand_get_redir_out(const scommand self){
    assert(scommand_inv(self));
    
    return self->redir_out;
}

char * scommand_to_string(const scommand self){
    assert(scommand_inv(self));
    
    GQueue* scmd_before_string = g_queue_copy(self->cmd); 
    
    char * scmd_to_string = NULL;
    
    while (!g_queue_is_empty(scmd_before_string)){
        char * copy_head = g_queue_pop_head(scmd_before_string);
        if(scmd_to_string == NULL){
            scmd_to_string = strdup(copy_head);
        }else{
            char * space = strmerge(scmd_to_string, " ");
            free(scmd_to_string);
            scmd_to_string = strmerge(space, copy_head);
            free(space);
        }      
    }
    if (self->redir_out != NULL){
        char * space = strmerge(scmd_to_string, "  >  ");
        free(scmd_to_string);
        scmd_to_string = strmerge(space, self->redir_out);
        free(space);
    } 
    if (self->redir_in != NULL){
        char * space = strmerge(scmd_to_string, "  <  ");
        free(scmd_to_string);
        scmd_to_string = strmerge(space, self->redir_in);
        free(space);
    } 
    if (scmd_to_string == NULL){
        scmd_to_string = strdup ("");
    }else if(strlen(scmd_to_string) > 0 && scmd_to_string[strlen(scmd_to_string) - 1] == ' '){
        scmd_to_string[strlen(scmd_to_string) -1] = '\0';
    }
    g_queue_free(scmd_before_string);
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

    pipeline_new->pipe = g_queue_new(); 
    pipeline_new->wait = true;

    assert(pipeline_inv(pipeline_new) && pipeline_is_empty(pipeline_new) && pipeline_get_wait(pipeline_new));

    return pipeline_new;
}

pipeline pipeline_destroy(pipeline self){
    assert(pipeline_inv(self));

    if(self->pipe != NULL){
        unsigned int pipeline_length  = g_queue_get_length(self->pipe);
        for(unsigned int pos = 0u; pos < pipeline_length; pos ++){
            scommand_destroy(g_queue_pop_tail(self->pipe));
        }
        g_queue_free(self->pipe);
    }

    free(self);
    self = NULL;
    return self;
}


/* Modificadores */

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


void pipeline_pop_front(pipeline self){
    assert(pipeline_inv(self) && !pipeline_is_empty(self));

    scommand kill = g_queue_pop_head(self->pipe);
    scommand_destroy(kill);
    kill = NULL;
}


void pipeline_set_wait(pipeline self, const bool w){
    assert(pipeline_inv(self));
    
    self->wait = w;
}


/* Proyectores */

bool pipeline_is_empty(const pipeline self){
    assert(pipeline_inv(self));

    bool pipe_is_empty = g_queue_is_empty(self->pipe);
    
    return pipe_is_empty;
}


unsigned int pipeline_length(const pipeline self){
    assert(pipeline_inv(self));
    
    unsigned int pipe_length = g_queue_get_length(self->pipe);

    return pipe_length;
}

scommand pipeline_front(const pipeline self){
    assert(pipeline_inv(self) && !pipeline_is_empty(self));

    scommand pipe_front = g_queue_peek_head(self->pipe);

    return pipe_front;
}

bool pipeline_get_wait(const pipeline self){
    assert(pipeline_inv(self));

    return self->wait;
}

char * pipeline_to_string(const pipeline self){
    assert(pipeline_inv(self));

    char * result = strdup("");
    char * temp;
    unsigned int length = pipeline_length(self);

    for (size_t i = 0; i < length; i++) {
        scommand sc = g_queue_peek_nth(self->pipe, i);
        char *scommand_string = scommand_to_string(sc);

        temp = strmerge(result , scommand_string);
        free(result);
        result = temp;

        if (i < length - 1){
            temp = strmerge(result, " | ");
            free(result);
            result = temp;
        }
        free(scommand_string);
    }

    if (!self->wait){
        temp = strmerge(result , " &");
        free(result);
        result = temp;
    }

    assert(pipeline_is_empty(self) || pipeline_get_wait(self) || strlen(result) > 0);

    return result;
}