#ifndef __ENV_H
#define __ENV_H

#include "value.h"
#include <Pigeon/string.h>

#define ENV_LEN 3000

typedef struct bucket_t {
    Value*  val;
    String  ident;
    bool    occupied;
    u32     hash;
} Bucket;

struct Env {
    Env* parent;
    Bucket* data;
    u64 len;
};

Env*     env_new(Env* parent);
void     env_free(Env* env);
Bucket   env_get(Env* env, String key);
void     env_insert(Env* env, String key, Value* val);
bool     env_has(Env* env, String key);
void     bucket_print(Bucket bucket);

#endif  //__ENV_H
