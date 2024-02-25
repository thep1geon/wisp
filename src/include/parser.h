#ifndef __PARSER_H
#define __PARSER_H

#include "ast.h"
#include "lexer.h"
#include <Pigeon/arena.h>

typedef struct parser_t {
    Lexer* lexer;
    AST* ast;
    Arena* arena;

    Token prev;
    Token curr;
    Token next;
} Parser;

Parser* parser_new(Arena* arena, Lexer* lexer);
void    parser_set_src(Parser* parser, String src);
void    parser_parse(Parser* parser);

#endif  //__PARSER_H
