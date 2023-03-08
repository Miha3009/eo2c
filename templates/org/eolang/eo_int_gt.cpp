#include "bool.h"

EO_object* eval_EO_int_gt(EO_object* obj) {
    bool value = ((EO_int*)get_parent(obj))->value > ((EO_int*)((EO_int_gt*)obj)->x)->value;
    EO_object* result = stack_alloc(sizeof(EO_int));
    init_EO_bool((EO_bool*)result, 0, value);
    return (EO_object*)result;
}