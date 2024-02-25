#include <Pigeon/defines.h>
#include <stdio.h>
#include <stdlib.h>
#include <Pigeon/arena.h>
#include <Pigeon/string.h>
#include <editline/readline.h>

#include "include/env.h"
#include "include/gc.h"
#include "include/wispfunc.h"
#include "include/value.h"
#include "include/ast.h"
#include "include/lexer.h"
#include "include/parser.h"

i32 main(i32 argc, char** argv) {
    Arena* arena = arena_new();
    String line = string("");

    Lexer* lexer;
    Parser* parser;

    Env* env = env_new(NULL);
    env_insert(env, string("+"),       &VALUE_NEW(VALUE_NATIVE, wisp_add));
    env_insert(env, string("-"),       &VALUE_NEW(VALUE_NATIVE, wisp_sub));
    env_insert(env, string("*"),       &VALUE_NEW(VALUE_NATIVE, wisp_mul));
    env_insert(env, string("/"),       &VALUE_NEW(VALUE_NATIVE, wisp_div));
    env_insert(env, string("="),       &VALUE_NEW(VALUE_NATIVE, wisp_eq));
    env_insert(env, string("/="),      &VALUE_NEW(VALUE_NATIVE, wisp_neq));
    env_insert(env, string("<"),       &VALUE_NEW(VALUE_NATIVE, wisp_lt));
    env_insert(env, string("<="),      &VALUE_NEW(VALUE_NATIVE, wisp_lte));
    env_insert(env, string(">"),       &VALUE_NEW(VALUE_NATIVE, wisp_gt));
    env_insert(env, string(">="),      &VALUE_NEW(VALUE_NATIVE, wisp_gte));
    env_insert(env, string("hello"),   &VALUE_NEW(VALUE_NATIVE, wisp_hello));
    env_insert(env, string("print"),   &VALUE_NEW(VALUE_NATIVE, wisp_print));
    env_insert(env, string("println"), &VALUE_NEW(VALUE_NATIVE, wisp_println));
    env_insert(env, string("set"),     &VALUE_NEW(VALUE_NATIVE, wisp_set));
    env_insert(env, string("car"),     &VALUE_NEW(VALUE_NATIVE, wisp_car));
    env_insert(env, string("cdr"),     &VALUE_NEW(VALUE_NATIVE, wisp_cdr));
    env_insert(env, string("range"),   &VALUE_NEW(VALUE_NATIVE, wisp_range));
    env_insert(env, string("t"),       &VALUE_NEW(VALUE_INTEGER, 1));

    Gc* gc = gc_new();

    if (argc < 2) {
        // gc_set_mode(gc, REPL);
        lexer = lexer_new(arena, string(""));
        parser = parser_new(arena, lexer);

        while (1) {
            if (gc->mode == AUTOMATIC ||
                gc->mode == REPL) {
                gc_collect(gc, env);
            }

            line.data = readline("Wisp> ");
            if (line.data == NULL) {
                ast_free(parser->ast);
                free(line.data);
                break;
            }

            add_history(line.data);

            parser_set_src(parser, line);
            parser_parse(parser);

            // ast_print(parser->ast);

            Value* val = ast_eval(parser->ast, env, gc);

            printf("\n");
            value_print(val);
            printf("\n");

            parser->ast = NULL;
            parser->lexer->line_number++;

            free(line.data);
        }
    }
    else {
        gc_set_mode(gc, INTERPRET);
        String file_path = string(argv[1]);
        String src = string_slurp_file_malloc(file_path);

        lexer = lexer_new(arena, src);
        parser = parser_new(arena, lexer);

        parser_parse(parser);
        ast_eval(parser->ast, env, gc);
        free(src.data);
    }

    env_free(env);
    gc_free(gc);
    arena_free(arena);
    return 0;
}
