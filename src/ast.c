#include <Pigeon/string.h>
#include <Pigeon/defines.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/ast.h"
#include "include/env.h"
#include "include/value.h"
#include "include/gc.h"

AST_Vec* ast_vec_new() {
    AST_Vec* vec = malloc(sizeof(AST_Vec));
    vec->cap = 64;
    vec->len = 0;

    vec->at = malloc(vec->cap * sizeof(AST*));
    return vec;
}

void ast_vec_append(AST_Vec* vec, AST* item) {
    if (vec->len >= vec->cap) {
        vec->cap = vec->cap == 0 ? 64 : vec->cap * 2;
        vec->at = realloc(vec->at, vec->cap * sizeof(AST*));
        ASSERT(vec->at, "Failed to realloc list");
    }
    vec->at[vec->len++] = item;
}

void ast_vec_free(AST_Vec* vec) {
    for (usize i = 0; i < vec->len; ++i) {
        ast_free(vec->at[i]);
    }

    free(vec->at);
    free(vec);
}

AST* ast_new(AST ast) {
    AST* ptr = (AST*)malloc(sizeof(AST));
    ptr->eval = true;
    if (ptr) *ptr = ast;
    return ptr;
}

void ast_print(AST* ast) {
    if (!ast) return;
    switch (ast->tag) {
        case AST_PROG: {
            struct AST_PROG data = ast->data.AST_PROG;
            printf("(");
            for (usize i = 0; i < data.vec->len; ++i) {
                ast_print(data.vec->at[i]);

                if (i + 1 < data.vec->len) {
                    printf(" ");
                }
            }
            printf(")");
            break;
        }
        case AST_LIST: {
            struct AST_LIST data = ast->data.AST_LIST;
            printf("(");
            for (usize i = 0; i < data.vec->len; ++i) {
                ast_print(data.vec->at[i]);

                if (i + 1 < data.vec->len) {
                    printf(" ");
                }
            }
            printf(")");
            break;
        }
        case AST_FN: {
            struct AST_FN data = ast->data.AST_FN;
            printf("(fn ");
            printf("(");
            ast_print(data.params);
            printf(") ");

            printf("(");
            for (usize i = 0; i < data.body->len; ++i) {
                ast_print(data.body->at[i]);

                if (i + 1 < data.body->len) {
                    printf(" ");
                }
            }
            
            printf(")");
            printf(")");

            break;
        }
        case AST_IF: {
            struct AST_IF data = ast->data.AST_IF;
            printf("(if ");
            printf("(");
            ast_print(data.condition);
            printf(") ");
            printf("(");
            ast_print(data.then_branch);
            printf(") ");
            printf("(");
            ast_print(data.else_branch);
            printf(") ");
            printf(")");

            break;
        }
        case AST_NUMBER: {
            struct AST_NUMBER data = ast->data.AST_NUMBER;
            printf("(INTEGER %d)", data.val);
            break;
        }
        case AST_SYM: {
            struct AST_SYM data = ast->data.AST_SYM;

            printf("(SYM ");
            string_print(data.sym);
            printf(")");
            break;
        }
        case AST_STR: {
            printf("(STR ");
            string_print(ast->data.AST_STR.str);
            printf(")");
            break;
        }
        case AST_NIL: {
            printf("(NIL)");
            break;
        }
    }
}

static Value* ast_eval_no_eval(AST* ast, Env* env, Gc* gc) {
    switch (ast->tag) {
        case AST_SYM: {
            struct AST_SYM data = ast->data.AST_SYM;

            String s = string_malloc_str(data.sym.data);

            Value* val = value_alloc(VALUE_NEW(VALUE_SYM, s), gc);
            return val;
        }
        case AST_LIST: {
            struct AST_LIST data = ast->data.AST_LIST;
            Value_Vec* vec = value_vec_new();

            for (usize i = 0; i < data.vec->len; ++i) {
                data.vec->at[i]->eval = false;
                value_vec_append(vec, ast_eval(data.vec->at[i], env, gc));
            }

            Value* val = value_alloc(VALUE_NEW(VALUE_LIST, vec), gc);
            return val;
        }

        case AST_STR:
        case AST_NUMBER: {
            ast->eval = true;
            Value* v = ast_eval(ast, env, gc);
            return v;
        }

        default: {
            Value* v = value_alloc(VALUE_NEW(VALUE_NIL, 0), gc);
            return v;
        }
    }
}

Value* ast_eval(AST* ast, Env* env, Gc* gc) {
    if (!ast->eval) {
        return ast_eval_no_eval(ast, env, gc);
    }

    switch (ast->tag) {
        case AST_PROG: {
            struct AST_PROG data = ast->data.AST_PROG;

            Value* v = value_alloc(VALUE_NEW(VALUE_NIL, 0), gc);
            for (usize i = 0; i < data.vec->len; ++i) {
                v = ast_eval(data.vec->at[i], env, gc);

                if (gc->mode == INTERPRET) {
                    gc_collect(gc, env);
                    gc_collect(gc, env);
                    gc_collect(gc, env);
                }
            }

            return v;
        }
        case AST_LIST: {
            struct AST_LIST data = ast->data.AST_LIST;

            // Treat the first symbol in a list as a function
            
            if (data.vec->len == 0) {
                Value_Vec* vec = value_vec_new();
                return value_alloc(VALUE_NEW(VALUE_LIST, vec), gc);
            }

            Value* sym_val = ast_eval(data.vec->at[0], env, gc);

            union {
                WispFunc native;
                Lambda* lambda;
            } func;
            if (sym_val->tag == VALUE_NATIVE) {
                func.native = VALUE_GET(sym_val, VALUE_NATIVE);
            }  else  if (sym_val->tag == VALUE_LAMBDA){
                func.lambda = VALUE_GET(sym_val, VALUE_LAMBDA);
            } else {
                ASSERT(false, "First symbol must be a function");
            }

            Value_Vec* vec = value_vec_new();
            for (usize i = 1; i < data.vec->len; ++i) {
                Value* val = ast_eval(data.vec->at[i], env, gc);
                value_vec_append(vec, val);
            }

            Value* v;
            if (sym_val->tag == VALUE_NATIVE) {
                v = func.native(gc, env, *vec);
            } else {
                v = lambda_call(func.lambda, gc, vec);
            }

            free(vec->at);
            free(vec);

            return v;
        }
        case AST_FN: {
            struct AST_FN data = ast->data.AST_FN;
            struct AST_LIST params = data.params->data.AST_LIST;
            AST* body = AST_NEW(AST_PROG, data.body);

            Lambda* lambda = calloc(1, sizeof(Lambda));
            lambda->argc = params.vec->len;
            lambda->body = body;
            lambda->env = env_new(env);
            lambda->params = calloc(1, sizeof(String) * params.vec->len);

            for (usize i = 0; i < lambda->argc; ++i) {
                lambda->params[i] = string_malloc_str(params.vec->at[i]->data.AST_SYM.sym.data);
            }

            Value* v = value_alloc(VALUE_NEW(VALUE_LAMBDA, lambda), gc);

            return v;
        }
        case AST_IF: {
            struct AST_IF data = ast->data.AST_IF;
            return value_alloc(VALUE_NEW(VALUE_NIL, 0), gc);
        }
        case AST_NUMBER: {
            struct AST_NUMBER data = ast->data.AST_NUMBER;
            i64 num = data.val;

            Value* v = value_alloc(VALUE_NEW(VALUE_INTEGER, num), gc);
            return v;
        }
        case AST_SYM: {
            struct AST_SYM data = ast->data.AST_SYM;

            if (!env_parent_has(env, data.sym)) {
                // TODO Figure out how to free all the memory
                // when an unknown function is called and the
                // program needs to crash
                Value *v = value_alloc(VALUE_NEW(VALUE_NIL, 0), gc);
                return v;
            }


            Bucket bucket = env_get(env, data.sym);
            Value* val = bucket.val;

            return val;
        }
        case AST_STR: {
            struct AST_STR data = ast->data.AST_STR;

            String s = string_malloc_str(data.str.data);

            Value* val = value_alloc(VALUE_NEW(VALUE_SYM, s), gc);
            return val;
        }
        case AST_NIL: {
            Value* val = value_alloc(VALUE_NEW(VALUE_NIL, 0), gc);
            return val;
        }
    }

    Value* val = value_alloc(VALUE_NEW(VALUE_NIL, 0), gc);
    return val;
}

void ast_free(AST* ast) {
    if (!ast) return;
    switch (ast->tag) {
        case AST_PROG: {
            struct AST_PROG data = ast->data.AST_PROG;
            ast_vec_free(data.vec);
            free(ast);
            return;
        }
        case AST_FN: {
            struct AST_FN data = ast->data.AST_FN;

            ast_free(data.params);
            free(ast);
            return;
        }
        case AST_IF: {
            struct AST_IF data = ast->data.AST_IF;

            ast_free(data.condition);
            ast_free(data.then_branch);
            ast_free(data.else_branch);
            free(ast);

            return;
        }
        case AST_LIST: {
            struct AST_LIST data = ast->data.AST_LIST;
            ast_vec_free(data.vec);
            free(ast);
            return;
        }
        case AST_NUMBER: {
            struct AST_NUMBER data = ast->data.AST_NUMBER;
            (void)data;
            free(ast);
            return;
        }
        case AST_SYM: {
            struct AST_SYM data = ast->data.AST_SYM;

            string_free(&data.sym);
            free(ast);
            return;
        }
        case AST_STR: {
            struct AST_STR data = ast->data.AST_STR;

            string_free(&data.str);
            free(ast);
            return;
        }
        case AST_NIL: {
            struct AST_NIL data = ast->data.AST_NIL;
            (void)data;
            free(ast);
            return;
        }
    }
}

AST* ast_clone(AST* ast) {
    switch (ast->tag) {
        case AST_NIL:
            return AST_NEW(AST_NIL, ast->data.AST_NIL.ptr);
        case AST_NUMBER:
            return AST_NEW(AST_NUMBER, ast->data.AST_NUMBER.val);
        case AST_SYM: {
            String copy = string_clone_malloc(ast->data.AST_SYM.sym);
            return AST_NEW(AST_SYM, copy);
        }
        case AST_STR: {
            String copy = string_clone_malloc(ast->data.AST_STR.str);
            return AST_NEW(AST_STR, copy);
        }
        case AST_LIST: {
            struct AST_LIST data = ast->data.AST_LIST;
            AST_Vec* old_vec = data.vec; 
            AST_Vec* new_vec = ast_vec_new();

            for (usize i = 0; i < old_vec->len; ++i) {
                AST* ast = ast_clone(old_vec->at[i]);
                ast_vec_append(new_vec, ast);
            }

            return AST_NEW(AST_LIST, new_vec);
        }

        case AST_PROG: {
            struct AST_PROG data = ast->data.AST_PROG;
            AST_Vec* old_vec = data.vec; 
            AST_Vec* new_vec = ast_vec_new();

            for (usize i = 0; i < old_vec->len; ++i) {
                AST* ast = ast_clone(old_vec->at[i]);
                ast_vec_append(new_vec, ast);
            }

            return AST_NEW(AST_PROG, new_vec);
        }

        case AST_FN: {
            struct AST_FN data = ast->data.AST_FN;
            
            AST_Vec* old_vec = data.body; 
            AST_Vec* new_vec = ast_vec_new();

            for (usize i = 0; i < old_vec->len; ++i) {
                AST* ast = ast_clone(old_vec->at[i]);
                ast_vec_append(new_vec, ast);
            }

            return AST_NEW(AST_FN, ast_clone(data.params), new_vec);
        }
        case AST_IF: {
            struct AST_IF data = ast->data.AST_IF; 
            return AST_NEW(AST_IF, 
                           ast_clone(data.condition), 
                           ast_clone(data.then_branch),
                           ast_clone(data.else_branch));
        }
    }

    // Let's not reach this, okay?
    return AST_NEW(AST_NIL, 0);
}
