#ifndef __AST_H
#define __AST_H

#include <Pigeon/defines.h>
#include <Pigeon/arena.h>
#include <Pigeon/string.h>
#include "value.h"

#define AST_NEW(tag, ...) \
    ast_new((AST){tag, {.tag=(struct tag){__VA_ARGS__}}, true}) 

typedef struct AST AST;

typedef struct ast_vec_t {
    AST** at;
    usize len; 
    usize cap; 
} AST_Vec;
AST_Vec* ast_vec_new();
void ast_vec_append(AST_Vec* vec, AST* item);
void ast_vec_free(AST_Vec* vec);

struct AST {
    enum {
        AST_NIL,
        AST_PROG,
        AST_LIST,
        AST_NUMBER,
        AST_SYM,
        AST_STR,
        AST_FN,
        AST_IF,
    } tag;

    union {
        struct AST_NIL
        { void* ptr; } AST_NIL;

        struct AST_PROG
        { AST_Vec* vec; } AST_PROG;

        struct AST_LIST
        { AST_Vec* vec; } AST_LIST;

        struct AST_NUMBER
        { i32 val; } AST_NUMBER;

        struct AST_SYM
        { String sym; } AST_SYM;

        struct AST_STR
        { String str; } AST_STR;

        struct AST_FN
        { AST* params; AST_Vec* body; } AST_FN;

        struct AST_IF
        { AST* condition; AST* then_branch; AST* else_branch; } AST_IF;
    } data;

    bool eval;
};

AST* ast_new(AST ast);
AST* ast_clone(AST* ast);
void ast_print(AST* ast);
Value* ast_eval(AST* ast, Env* env, Gc* gc);
void ast_free(AST* ast);

#endif  //__AST_H
