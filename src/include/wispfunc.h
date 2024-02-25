#ifndef __WISP_FUNC_H
#define __WISP_FUNC_H

#include "value.h"

Value* wisp_add     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_sub     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_mul     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_div     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_eq      (Gc* gc, Env* env, Value_Vec args);
Value* wisp_neq     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_lt      (Gc* gc, Env* env, Value_Vec args);
Value* wisp_lte     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_gt      (Gc* gc, Env* env, Value_Vec args);
Value* wisp_gte     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_hello   (Gc* gc, Env* env, Value_Vec args);
Value* wisp_print   (Gc* gc, Env* env, Value_Vec args);
Value* wisp_println (Gc* gc, Env* env, Value_Vec args);
Value* wisp_set     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_defun   (Gc* gc, Env* env, Value_Vec args);
Value* wisp_range   (Gc* gc, Env* env, Value_Vec args);
Value* wisp_car     (Gc* gc, Env* env, Value_Vec args);
Value* wisp_cdr     (Gc* gc, Env* env, Value_Vec args);

#endif  //__WISP_FUNC_H

