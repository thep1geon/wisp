#include "include/value.h"
#include "include/ast.h"
#include "include/env.h"
#include "include/gc.h"
#include <Pigeon/string.h>
#include <stdio.h>
#include <stdlib.h>


Value_Vec* value_vec_new() {
    Value_Vec* vec = malloc(sizeof(Value_Vec));
    vec->at = malloc(sizeof(Value*) * 128);
    vec->cap = 128;
    vec->len = 0;
    return vec;
}

void value_vec_free(Value_Vec* vec) {
    for (usize i = 0; i < vec->len; ++i) {
        value_free(vec->at[i]);
    }

    free(vec->at);
    free(vec);
}

void value_vec_append(Value_Vec* vec, Value* item) {
    if (vec->len >= vec->cap) {
        vec->cap = vec->cap == 0 ? DATA_START_CAPACITY : vec->cap*2;
        vec->at = realloc(vec->at, vec->cap*sizeof(Value*));
        ASSERT(vec->at != NULL, "outta ram");
    }
    vec->at[vec->len++] = (item);
}

Value* lambda_call(Lambda* lambda, Gc* parent_gc, Env* parent, Value_Vec* args) {
    Gc* gc = gc_new();
    Env* env = env_new(parent);
    gc_set_mode(gc, REPL);

    if (lambda->argc != args->len) {
        return value_alloc(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    for (usize i = 0; i < args->len; ++i) {
        env_insert(env, lambda->params[i], args->at[i]);
    }

    Value* v = ast_eval(lambda->body, env, gc);
    Value* ret_val = value_alloc(*v, parent_gc);

    env_free(env);    
    gc_free(gc);

    return ret_val;
}

Value* value_alloc(Value val, Gc* gc) {
    Value* v = malloc(sizeof(Value));
    if (v) *v = val;

    if (gc != NULL) {
        gc_add_value(gc, v);
    }

    return v;
}

void value_print(Value* val) {
    switch (val->tag) {
        case VALUE_INTEGER: {
            printf("%ld", VALUE_GET(val, VALUE_INTEGER));
            break;
        }
        case VALUE_REAL: {
            printf("%f", VALUE_GET(val, VALUE_REAL));
            break;
        }
        case VALUE_STR: {
            string_print(VALUE_GET(val, VALUE_STR));
            break;
        }
        case VALUE_SYM: {
            string_print(VALUE_GET(val, VALUE_SYM));
            break;
        }
        case VALUE_NIL: {
            printf("nil");
            break;
        }
        case VALUE_ERR: {
            String msg = val->val.VALUE_ERR.val.msg;
            printf("(Err ");
            string_print(msg);
            printf(")");
            break;
        }
        case VALUE_LIST: {
            Value_Vec* list = VALUE_GET(val, VALUE_LIST);
            printf("(");
            for (usize i = 0; i < list->len; ++i) {
                value_print(list->at[i]);

                if (i + 1 < list->len) {
                    printf(" ");
                }
            }

            printf(")");
            break;
        }
        case VALUE_NATIVE: {
            printf("(WispFuc) @ %p", *(void**)&VALUE_GET(val, VALUE_NATIVE));
            break;
        }

        case VALUE_LAMBDA: {
            printf("(Lambda) @ %p", (void*)&VALUE_GET(val, VALUE_LAMBDA));
            break;
        }
    }
}

void value_free(Value* val) {
    switch (val->tag) {
        case VALUE_ERR:
            string_free(&val->val.VALUE_ERR.val.msg);
            goto __value_free;
        case VALUE_SYM:
            string_free(&val->val.VALUE_SYM.val);
            goto __value_free;
        case VALUE_STR:
            string_free(&val->val.VALUE_STR.val);
            goto __value_free;

        case VALUE_NIL:
        case VALUE_INTEGER:
        case VALUE_NATIVE:
        case VALUE_REAL:
            goto __value_free;

        case VALUE_LIST: {
            Value_Vec* vec = VALUE_GET(val, VALUE_LIST);

            // for (usize i = 0; i < vec->len; ++i) {
            //     value_free(vec->at[i]);
            // }

            free(vec->at);
            free(vec);

            goto __value_free;
        }

        case VALUE_LAMBDA: {
            Lambda lambda = VALUE_GET(val, VALUE_LAMBDA);

            ast_free(lambda.body);
            
            for (usize i = 0; i < lambda.argc; ++i) {
                string_free(&lambda.params[i]);
            }

            free(lambda.params);

            goto __value_free;
        }

    }

__value_free:
    free(val);
    return;
}

void value_mark(Value* val) {
    switch (val->tag) {
        case VALUE_SYM:
        case VALUE_STR:
        case VALUE_NIL:
        case VALUE_INTEGER:
        case VALUE_NATIVE:
        case VALUE_REAL:
        case VALUE_ERR:
        case VALUE_LAMBDA:
            val->marked = true;
            return;

        case VALUE_LIST: {
            Value_Vec *vec = VALUE_GET(val, VALUE_LIST);
            for (usize i = 0; i < vec->len; ++i) {
                value_mark(vec->at[i]);
            }
            val->marked = true;
            return; 
        }
    }
}

void value_unmark(Value* val) {
    switch (val->tag) {
        case VALUE_SYM:
        case VALUE_STR:
        case VALUE_NIL:
        case VALUE_INTEGER:
        case VALUE_NATIVE:
        case VALUE_REAL:
        case VALUE_LAMBDA:
        case VALUE_ERR:
            val->marked = false;
            return;

        case VALUE_LIST: {
            Value_Vec* vec = VALUE_GET(val, VALUE_LIST);
            for (usize i = 0; i < vec->len; ++i) {
                value_unmark(vec->at[i]);
            } 
            val->marked = false;
        }
    }
}
