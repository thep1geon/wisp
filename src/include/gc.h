#ifndef __GC_H
#define __GC_H

#include "value.h"

#define GC_INITIAL_CAP 128

typedef enum {
    AUTOMATIC,
    REPL,
    INTERPRET,
    OFF,
} GcMode;

struct Gc {
    GcMode mode;
    Value** at;
    u64 len;
    u64 cap;
};

Gc* gc_new();
void gc_free(Gc* gc);
const char* gc_get_mode(Gc* gc);
void gc_set_mode(Gc* gc, GcMode mode);
void gc_add_value(Gc* gc, Value* val);
void gc_collect(Gc* gc, Env* env);
void gc_inspect(Gc* gc);

#endif  //__GC_H
