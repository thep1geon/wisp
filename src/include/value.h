#ifndef __VALUE_H
#define __VALUE_H

#include <Pigeon/defines.h>
#include <Pigeon/string.h>

#define VALUE_NEW(tag, ...) \
    (Value){tag, {.tag=(struct tag){__VA_ARGS__}}, .marked = false}

#define VALUE_GET(V, tag) ((V)->val.tag.val)

typedef struct Value Value;
typedef struct AST AST;
typedef struct Value_Vec Value_Vec;
typedef struct Env Env;
typedef struct Gc Gc;
typedef struct Lambda Lambda;
typedef struct Err Err;

typedef Value*(*WispFunc)(Gc* gc, Env* env, Value_Vec args);

struct Value_Vec {
    Value** at;
    u64    len;
    u64    cap;
};
Value_Vec* value_vec_new();
void value_vec_free(Value_Vec* vec);
void value_vec_append(Value_Vec* vec, Value* item);

struct Lambda {
    String* params;
    u32 argc;
    AST* body;
};
Value* lambda_call(Lambda* lambda, Gc* parent_gc, Env* parent, Value_Vec* args);

struct Err {
    enum {
        NONE,   
        ERR,
    } ErrType;

    String msg; 
};

struct Value {
    enum {
        VALUE_INTEGER,
        VALUE_REAL,
        VALUE_STR,
        VALUE_SYM,
        VALUE_NATIVE,
        VALUE_LIST,
        VALUE_LAMBDA,
        VALUE_NIL,
        VALUE_ERR,
    } tag;

    union {
        struct VALUE_INTEGER
        { i64 val; } VALUE_INTEGER;

        struct VALUE_REAL
        { f64 val; } VALUE_REAL;

        struct VALUE_STR
        { String val; } VALUE_STR;
        
        struct VALUE_SYM
        { String val; } VALUE_SYM;

        struct VALUE_LIST
        { Value_Vec* val; } VALUE_LIST;
        
        struct VALUE_NATIVE
        { WispFunc val; } VALUE_NATIVE;
        
        struct VALUE_LAMBDA
        { Lambda val; } VALUE_LAMBDA;

        struct VALUE_NIL
        { u8 val; } VALUE_NIL;

        struct VALUE_ERR
        { Err val; } VALUE_ERR;
    } val;

    bool marked;
};

Value* value_alloc(Value val, Gc* gc);
void value_print(Value* val);
void value_mark(Value* val);
void value_unmark(Value* val);
void value_free(Value* val);

#endif  //__VALUE_H
