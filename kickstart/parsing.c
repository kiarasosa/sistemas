#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"

// Función para parsear un scommand
static scommand parse_scommand(Parser p) {
    assert(p != NULL); 
    scommand nuevo = scommand_new(); 
    arg_kind_t arg_aux; // Variable para almacenar el tipo de argumento
    parser_skip_blanks(p); // Omite espacios en blanco en el parser
    char * string_aux = parser_next_argument(p, &arg_aux); // Obtiene el siguiente argumento y su tipo
    
    while(string_aux != NULL){
        
        if (arg_aux == ARG_NORMAL){
            scommand_push_back(nuevo, string_aux); // Agrega un argumento normal al comando
        } 
        else if (arg_aux == ARG_INPUT){
            scommand_set_redir_in(nuevo, string_aux); // Establece la redirección de entrada
        } 
        else if (arg_aux == ARG_OUTPUT){
            scommand_set_redir_out(nuevo, string_aux); // Establece la redirección de salida
        }
        
        string_aux = parser_next_argument(p, &arg_aux); // Obtiene el siguiente argumento
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
    bool error = false, another_pipe = true; // Flags para controlar errores y presencia de más pipes
    
    while (another_pipe && !error) {
        cmd = parse_scommand(p); 
        error = (cmd == NULL); 
        if (!error){
            parser_skip_blanks(p); 
            pipeline_push_back(result, cmd); // Agrega el comando al pipeline
            parser_op_pipe(p, &another_pipe); // Verifica si hay otro pipe 
        }
         
    }
    
    if(!parser_at_eof(p)){ // Si no se llegó al final del parser
        bool is_background;
        parser_op_background(p, &is_background); // Verifica si el pipeline debe ejecutarse en background
        pipeline_set_wait(result, !is_background); 
        
        bool garbage;
        parser_garbage(p, &garbage); // Verifica si hay caracteres no esperados (basura) después del pipeline
        if(garbage){
            parser_last_garbage(p); // Procesa la basura encontrada
            parser_destroy(p); // Destruye el parser
            return NULL; 
        }
        if (error || pipeline_length(result) == 0){
            result = pipeline_destroy(result); 
            result = NULL; 
        }
    }

    return result; 
}
