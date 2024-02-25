#include "include/wispfunc.h"
#include "include/env.h"
#include "include/value.h"
#include <Pigeon/defines.h>
#include <stdio.h>

#define RETURN_VALUE(V, gc) do {\
    Value* v =  value_alloc(V, gc);\
    return v;\
} while (0);

Value* wisp_add(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len == 0) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    i64 acc = 0;

    for (usize i = 0; i < args.len; ++i) {
        acc += VALUE_GET(args.at[i], VALUE_INTEGER);
    }

    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, acc), gc);
}

Value* wisp_sub(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len == 0) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    i64 acc = VALUE_GET(args.at[0], VALUE_INTEGER);

    if (args.len == 1) {
        RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, -1*acc), gc);
    }

    for (usize i = 1; i < args.len; ++i) {
        acc -= VALUE_GET(args.at[i], VALUE_INTEGER);
    }

    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, acc), gc);
}

Value* wisp_div(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len == 0) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    f64 top = VALUE_GET(args.at[0], VALUE_INTEGER);
    f64 bottom = 1;

    for (usize i = 1; i < args.len; ++i) {
        bottom *= VALUE_GET(args.at[i], VALUE_INTEGER);
    }

    RETURN_VALUE(VALUE_NEW(VALUE_REAL, top / bottom), gc);
}

Value* wisp_mul(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len == 0) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    i64 acc = VALUE_GET(args.at[0], VALUE_INTEGER);

    for (usize i = 1; i < args.len; ++i) {
        acc *= VALUE_GET(args.at[i], VALUE_INTEGER);
    }

    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, acc), gc);
}

Value* wisp_hello(Gc* gc, Env* env, Value_Vec args) {
    (void)args;
    (void)env;
    printf("Hello from C!\n");
    RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
}

Value* wisp_print(Gc* gc, Env* env, Value_Vec args) {
    (void)env;
    for (usize i = 0; i < args.len; ++i) {
        value_print(args.at[i]);

        if (i + 1 < args.len) {
            printf(" ");
        }
    }

    RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
}

Value* wisp_println(Gc* gc, Env* env, Value_Vec args) {
    (void)env;
    for (usize i = 0; i < args.len; ++i) {
        value_print(args.at[i]);

        printf("\n");
    }

    RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
}

Value* wisp_set(Gc* gc, Env* env, Value_Vec args) {
    if (args.len != 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    String var = VALUE_GET(args.at[0], VALUE_SYM);
    env_insert(env, var, args.at[1]);
     
    RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
}

Value* wisp_eq(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len < 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    } 

    i64 a_val;
    i64 b_val;

    for (usize i = 1; i < args.len; ++i) {
        Value* a = args.at[i-1];
        Value* b = args.at[i];

        a_val = VALUE_GET(a, VALUE_INTEGER);
        b_val = VALUE_GET(b, VALUE_INTEGER);

        if (a_val != b_val) {
            RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
        }
    }
    
    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, 1), gc);
}

Value* wisp_neq(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len < 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    } 

    i64 a_val;
    i64 b_val;

    for (usize i = 1; i < args.len; ++i) {
        Value* a = args.at[i-1];
        Value* b = args.at[i];

        a_val = VALUE_GET(a, VALUE_INTEGER);
        b_val = VALUE_GET(b, VALUE_INTEGER);

        if (a_val == b_val) {
            RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
        }
    }
    
    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, 1), gc);
}

Value* wisp_lt(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len < 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    } 

    i64 a_val;
    i64 b_val;

    for (usize i = 1; i < args.len; ++i) {
        Value* a = args.at[i-1];
        Value* b = args.at[i];

        a_val = VALUE_GET(a, VALUE_INTEGER);
        b_val = VALUE_GET(b, VALUE_INTEGER);

        if (a_val >= b_val) {
            RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
        }
    }

    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, 1), gc);
}

Value* wisp_lte(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len < 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    } 

    i64 a_val;
    i64 b_val;

    for (usize i = 1; i < args.len; ++i) {
        Value* a = args.at[i-1];
        Value* b = args.at[i];

        a_val = VALUE_GET(a, VALUE_INTEGER);
        b_val = VALUE_GET(b, VALUE_INTEGER);

        if (a_val > b_val) {
            RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
        }
    }

    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, 1), gc);
}

Value* wisp_gt(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len < 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    } 

    i64 a_val;
    i64 b_val;

    for (usize i = 1; i < args.len; ++i) {
        Value* a = args.at[i-1];
        Value* b = args.at[i];

        a_val = VALUE_GET(a, VALUE_INTEGER);
        b_val = VALUE_GET(b, VALUE_INTEGER);

        if (a_val <= b_val) {
            RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
        }
    }
    
    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, 1), gc);
}

Value* wisp_gte(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len < 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    } 

    i64 a_val;
    i64 b_val;

    for (usize i = 1; i < args.len; ++i) {
        Value* a = args.at[i-1];
        Value* b = args.at[i];

        a_val = VALUE_GET(a, VALUE_INTEGER);
        b_val = VALUE_GET(b, VALUE_INTEGER);

        if (a_val < b_val) {
            RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
        }
    }
    
    RETURN_VALUE(VALUE_NEW(VALUE_INTEGER, 1), gc);
}

Value* wisp_range(Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len < 2) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    i64 bottom = VALUE_GET(args.at[0], VALUE_INTEGER);
    i64 top = VALUE_GET(args.at[1], VALUE_INTEGER);

    i64 step = 1;

    if (args.len == 3) {
        step = VALUE_GET(args.at[2], VALUE_INTEGER);
    }

    Value_Vec* vec = value_vec_new();

    for (i64 i = bottom; i < top; i += step) {
        Value* number = value_alloc(VALUE_NEW(VALUE_INTEGER, i), gc);
        value_vec_append(vec, number);
    }

    RETURN_VALUE(VALUE_NEW(VALUE_LIST, vec), gc);
}

Value* wisp_car (Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len != 1) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }


    Value_Vec* vec = VALUE_GET(args.at[0], VALUE_LIST);

    RETURN_VALUE(*vec->at[0], gc);
}

Value* wisp_cdr (Gc* gc, Env* env, Value_Vec args) {
    (void)env;

    if (args.len != 1 || args.at[0]->tag != VALUE_LIST) {
        RETURN_VALUE(VALUE_NEW(VALUE_NIL, 0), gc);
    }

    Value_Vec* list = VALUE_GET(args.at[0], VALUE_LIST);
    Value_Vec* vec = value_vec_new();

    for (usize i = 1; i < list->len; ++i) {
        value_vec_append(vec, list->at[i]);
    }

    RETURN_VALUE(VALUE_NEW(VALUE_LIST, vec), gc);
}
