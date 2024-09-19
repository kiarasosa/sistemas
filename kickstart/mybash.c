#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

#include "obfuscated.h"

#include "obfuscated.h"

static void show_prompt(void) {
    printf ("mybash> ");
    fflush (stdout);
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input = parser_new(stdin);

        while (!parser_at_eof(input)) {
        show_prompt();
        pipe = parse_pipeline(input);

        if (pipe != NULL) {
            execute_pipeline(pipe);
            pipeline_destroy(pipe);
        }

    }
    parser_destroy(input); input = NULL;
    return EXIT_SUCCESS;
}
