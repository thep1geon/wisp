#include "include/env.h"
#include <Pigeon/string.h>
#include <stdio.h>
#include <stdlib.h>

static u64 hash(String key) {
    u64 hash = 0; 

    for (u64 i = 0; i < key.len; ++i) {
        // hash = (hash + (u64)*key.data+i) << i;
        hash = (hash + (u64)*key.data+i) << (5 * i);
        hash += key.len * i;
    }

    hash %= ENV_LEN;

    return hash;
}

Env* env_new(Env* parent) {
    Env* env = malloc(sizeof(Env));
    env->len = ENV_LEN;
    env->data = malloc(ENV_LEN * sizeof(Bucket));
    env->parent = parent;

    for (usize i = 0; i < ENV_LEN; ++i) {
        env->data[i] = (Bucket) {NULL, string(""), false, 0};
    }

    return env;
}

void env_free(Env* env) {
    if (env) {
        free(env->data);
        free(env);
    } 
}


i64 env_get_hash(Env* env, String key) {
    u64 hash_val = hash(key); 
    u64 first_hash = hash_val;
    Bucket bucket = env->data[hash_val];

    if (bucket.occupied && string_eq(key, bucket.ident)) {
        return hash_val;
    }

    while (!string_eq(bucket.ident, key)) {
        hash_val = (hash_val + 1) % ENV_LEN;

        if (hash_val == first_hash) {
            // Return -1 if the hashmap doesn't have the key
            return -1;
        }

        bucket = env->data[hash_val];
    }

    return hash_val;
}

Bucket env_get(Env* env, String key) {
    if (env_has(env, key)) {
        return env->data[env_get_hash(env, key)];
    }

    if (env->parent == NULL) {
        return (Bucket) {.occupied = 0};
    }
    
    return env_get(env->parent, key);
}

static bool env_insert_index(Env* env, u64 index, String key, Value* val) {
    Bucket* bucket = &env->data[index];
    
    if (bucket->occupied && !string_eq(bucket->ident, key)) {
        return false; 
    }

    *bucket = (Bucket){
        .val = val, 
        .ident = key, 
        .occupied = true,
        .hash = index,
    };

    return true;
}

void env_insert(Env* env, String key, Value* val) {
    u64 hash_val = hash(key);

    while (!env_insert_index(env, hash_val, key, val)) {
        hash_val = (hash_val + 1) % ENV_LEN;
    }
}

bool env_parent_has(Env* env, String key) {
    i64 hash_val = env_get_hash(env, key);

    if (hash_val < 0) {
        if (env->parent == NULL) {
            return false;
        }

        return env_parent_has(env->parent, key);
    }

    return true;
}

bool env_has(Env* env, String key) {
    i64 hash_val = env_get_hash(env, key);
    return hash_val >= 0;
}

void env_print(Env* env) {
    for (usize i = 0; i < env->len; ++i) {
        Bucket b = env->data[i];

        printf("[%zu] ", i);
        bucket_print(b);
    }
}

void bucket_print(Bucket b) {
    if (!b.occupied) {
        printf("Bucket: { Empty }\n");
        return;
    }

    printf("Bucket: {Key: %s, Value: ", b.ident.data);
    value_print(b.val);
    printf(", hash: %d}\n", b.hash);
}
