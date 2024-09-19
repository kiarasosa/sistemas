#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"

static scommand parse_scommand(Parser p) {
    assert(p != NULL);
    scommand nuevo = scommand_new();
    arg_kind_t arg_aux;
    parser_skip_blanks(p);
    char * string_aux = parser_next_argument(p, &arg_aux);
    
    while(string_aux != NULL){
        
        if (arg_aux == ARG_NORMAL){
            scommand_push_back(nuevo, string_aux);
        } 
        else if (arg_aux == ARG_INPUT){
            scommand_set_redir_in(nuevo, string_aux);
        } 
        else if (arg_aux == ARG_OUTPUT){
            scommand_set_redir_out(nuevo, string_aux);
        }
        
        string_aux = parser_next_argument(p, &arg_aux);
        }
       
        if(arg_aux == ARG_INPUT || arg_aux == ARG_OUTPUT || scommand_is_empty(nuevo)){  
            scommand_destroy(nuevo);
            return NULL;
        }

    return nuevo;
}

pipeline parse_pipeline(Parser p) {
    assert(p != NULL && !parser_at_eof(p));

    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error = false, another_pipe = true;
    
    while (another_pipe && !error) {
        cmd = parse_scommand(p);
        error = (cmd == NULL); 
        if (!error){
            parser_skip_blanks(p);
            pipeline_push_back(result, cmd);
            parser_op_pipe(p, &another_pipe);
        }
         
    }
    
    if(!parser_at_eof(p)){
    bool is_background;
    parser_op_background(p, &is_background);
    pipeline_set_wait(result, !is_background);
    
    bool garbage;
    parser_garbage(p, &garbage);
    if(garbage){
        parser_last_garbage(p);
        parser_destroy(p);
        return NULL;
    }
    if (error || pipeline_length(result) == 0){
        result = pipeline_destroy(result);
        result = NULL;
    }
    }

    return result; 
}

