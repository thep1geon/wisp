#include "include/gc.h"
#include "include/env.h"
#include "include/value.h"
#include <stdio.h>
#include <stdlib.h>

Gc* gc_new() {
    Gc* gc = malloc(sizeof(Gc));
    gc->at = malloc(sizeof(Value*) * GC_INITIAL_CAP);
    gc->cap = GC_INITIAL_CAP;
    gc->len = 0;
    gc->mode = AUTOMATIC;

    return gc;
}

void gc_free(Gc* gc) {
    for (usize i = 0; i < gc->len; ++i) {
        value_free(gc->at[i]);
    }

    free(gc->at);
    free(gc);
}

const char* gc_get_mode(Gc* gc) {
    switch (gc->mode) {
        case OFF: return "OFF";
        case AUTOMATIC: return "AUTOMATIC";
        case REPL: return "REPL";
        case INTERPRET: return "INTERPRET";
    }

    return "Unknown GC mode :P";
}

void gc_set_mode(Gc* gc, GcMode mode) {
    gc->mode = mode;
}

void gc_add_value(Gc* gc, Value* val) {
    if (gc->len >= gc->cap) {
        gc->cap *= 2;
        gc->at = realloc(gc->at, gc->cap * sizeof(Value));

        for (usize i = gc->len+1; i < gc->cap; ++i) {
            *gc->at[i] = VALUE_NEW(VALUE_NIL, 0);
        }

        ASSERT(gc->at, "GC failed to add new value");
    }

    gc->at[gc->len++] = val;
}

static int compare_value(const void* a, const void* b) {
    i64 d = ((const Value*)b - (const Value*)a);

    if (d < 0) {
        return -1;
    } 
    else if (d > 0) {
        return 1;
    }
    else {
        return 0;
    }
}

void gc_collect(Gc* gc, Env* env) {
    // TODO sort and defragment the array of values

    qsort(gc->at, gc->len, sizeof(Value*), compare_value);

    while(gc->len > 0 && gc->at[gc->len - 1]->tag == VALUE_NIL) {
        free(gc->at[gc->len - 1]);
        gc->len--;
    }

    for (usize i = 0; i < env->len; ++i) {
        Bucket bucket = env->data[i]; 
        if (!bucket.occupied) continue;

        value_mark(bucket.val);
    }

    for (usize i = 0; i < gc->len; ++i) {
        if (gc->at[i]->tag == VALUE_SYM) {
            String sym = VALUE_GET(gc->at[i], VALUE_SYM);
            if (env_has(env, sym)) {
                value_mark(gc->at[i]);
            }
        }

        if (!gc->at[i]->marked) {
            value_free(gc->at[i]);
            gc->at[i] = value_alloc(VALUE_NEW(VALUE_NIL, 0));
        }
    }

    for (usize i = 0; i < env->len; ++i) {
        Bucket bucket = env->data[i]; 
        if (!bucket.occupied) continue;

        value_unmark(bucket.val);
    }
}

void gc_inspect(Gc* gc) {
    for (usize i = 0; i < gc->len; ++i) {
        Value* v = gc->at[i];
        if (v->tag == VALUE_NIL) {
            printf(".");
        } else {
            printf("+");
        }
    }
    printf("\n");
}

