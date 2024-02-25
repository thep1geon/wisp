#include "include/value.h"
#include "include/ast.h"
#include "include/env.h"
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
    DA_APPEND(vec, item);
}

Value* value_alloc(Value val) {
    Value* v = malloc(sizeof(Value));
    if (v) *v = val;
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
            string_print(VALUE_GET(val, VALUE_STR));
            break;
        }
        case VALUE_NIL: {
            printf("nil");
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
            printf("(WispFuc) @ %p", (void*)&VALUE_GET(val, VALUE_LAMBDA));
            break;
        }
    }
}

void value_free(Value* val) {
    switch (val->tag) {
        case VALUE_SYM:
            string_free(val->val.VALUE_SYM.val);
            goto __value_free;
        case VALUE_STR:
            string_free(val->val.VALUE_STR.val);
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
            env_free(lambda.env);
            ast_free(lambda.body);
            
            for (usize i = 0; i < lambda.argc; ++i) {
                string_free(lambda.params[i]);
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
        case VALUE_LAMBDA:
            val->marked = true;
            return;

        case VALUE_LIST: {
            Value_Vec* vec = VALUE_GET(val, VALUE_LIST);
            for (usize i = 0; i < vec->len; ++i) {
                value_mark(vec->at[i]);
            } 
            val->marked = true;
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
