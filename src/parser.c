#include "include/parser.h"
#include "include/ast.h"
#include "include/lexer.h"
#include <Pigeon/arena.h>
#include <Pigeon/defines.h>
#include <Pigeon/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void parser_advance(Parser* p);
static AST* parse_form(Parser* p);
static AST* parse_atom(Parser* p);
static AST* parse_sym(Parser* p);
static AST* parse_number(Parser* p);
static AST* parse_str(Parser* p);
static AST* parse_list(Parser* p);
static AST* parse_fn(Parser* p);
static AST* parse_def(Parser* p);
static AST* parse_let(Parser* p);


static void parser_advance(Parser* p) {
    string_free(&p->prev.lexeme);

    p->prev = p->curr;
    p->curr = p->next;
    p->next = lexer_next_token(p->lexer);
}

Parser* parser_new(Arena* arena, Lexer* lexer) {
    Parser* p = arena_alloc(arena, sizeof(Parser));
    p->arena = arena;
    p->lexer = lexer;
    p->ast = AST_NEW(AST_PROG, ast_vec_new());

    parser_advance(p);
    parser_advance(p);
    return p;
}

void parser_set_src(Parser* parser, String src) {
    ast_free(parser->ast);
    parser->ast = AST_NEW(AST_PROG, ast_vec_new());
    parser->lexer->src = src;
    parser->lexer->cursor = 0;
    parser->lexer->column = 0;
    parser_advance(parser);
    parser_advance(parser);
}

static AST* parse_fn(Parser* p) {
    parser_advance(p);
    AST* params = parse_list(p);
    parser_advance(p);
    if (params->tag != AST_LIST) {
        ast_free(p->ast);
        free(params);
        printf("Invalid tokens following fn name");
    }

    AST_Vec* body = ast_vec_new();
    while (p->curr.type != Token_RParen) {
        ast_vec_append(body, parse_form(p));
    }
    parser_advance(p);

    return AST_NEW(AST_FN, params, body);
}

static AST* parse_def(Parser* p) {
    parser_advance(p);
    (void)p;
    return AST_NEW(AST_NIL, 0);
}

static AST* parse_let(Parser* p) {
    parser_advance(p);
    (void)p;
    return AST_NEW(AST_NIL, 0);
}

static AST* parse_number(Parser* p) {
    return AST_NEW(AST_NUMBER, string_to_number(p->curr.lexeme));
}

static AST* parse_sym(Parser* p) {
    String s = string_malloc_str(p->curr.lexeme.data);
    return AST_NEW(AST_SYM, s);
}
static AST* parse_str(Parser* p) {
    String s = string_malloc_str(p->curr.lexeme.data);
    return AST_NEW(AST_STR, s);
}

static AST* parse_atom(Parser* p) {
    switch (p->curr.type) {
        case Token_Number: {
            return parse_number(p);
        } 
        case Token_Plus:
        case Token_Dash:
        case Token_Star:
        case Token_Slash:
        case Token_Eq:
        case Token_Greater:
        case Token_GreaterEq:
        case Token_Less:
        case Token_LessEq:
        case Token_Neq:
        case Token_Symbol:
            return parse_sym(p);
        case Token_String: {
            return parse_str(p);
        }

        case Token_Tick: {
            parser_advance(p);
            AST* ast = parse_form(p);
            ast->eval = false;
            return ast;
        };
        default: {
            token_loc_print(p->curr);
            puts("(Unknown Token)");
            return AST_NEW(AST_NIL, NULL);
        }
    }
}

static AST* parse_list(Parser* p) {
    AST* list = AST_NEW(AST_LIST, ast_vec_new());
    struct AST_LIST data = list->data.AST_LIST;

    while (p->next.type != Token_RParen) {
        AST* item;
        parser_advance(p);
        if (p->curr.type == Token_LParen) {
            item = parse_form(p);
        }
        else if (p->curr.type == Token_EOF) {
            string_free(&p->prev.lexeme);
            string_free(&p->curr.lexeme);
            string_free(&p->next.lexeme);

            ast_free(list);
            ast_free(p->ast);
            token_loc_print(p->curr);
            arena_free(p->arena);
            ASSERT(0, "Unbalanced Parens");
            break;
        }
        else {
            item = parse_atom(p);
            if (item->tag == AST_NIL) {
                free(item);
                break;
            }
        }

        ast_vec_append(data.vec, item);
    }

    parser_advance(p);
    return list;
}

static AST* parse_form(Parser* p) {
    if (p->curr.type == Token_LParen) {
        if (p->next.type == Token_Symbol) {
            String sym = p->next.lexeme;
            if (string_eq(sym, string("def"))) {
                parser_advance(p);
                return parse_def(p);
            }
            else if (string_eq(sym, string("fn"))) {
                parser_advance(p);
                return parse_fn(p);
            }
            else if (string_eq(sym, string("let"))) {
                parser_advance(p);
                return parse_let(p);
            }  
        }

        return parse_list(p);
    }

    return parse_atom(p);
}

void parser_parse(Parser* parser) {
    AST_Vec* vec = parser->ast->data.AST_PROG.vec;
    while (parser->curr.type != Token_EOF) {
        ast_vec_append(vec, parse_form(parser));
        parser_advance(parser);
    }

    string_free(&parser->prev.lexeme);
    string_free(&parser->curr.lexeme);
    string_free(&parser->next.lexeme);
}

