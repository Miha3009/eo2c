#include "bool.h"

EO_object* eval_EO_float_gt(EO_object* obj) {
    bool value = ((EO_float*)get_parent(obj))->value > ((EO_float*)((EO_float_gt*)obj)->x)->value;
    EO_object* result = stack_alloc(sizeof(EO_float));
    init_EO_bool((EO_bool*)result, 0, value);
    return (EO_object*)result;
}