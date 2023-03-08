#include "int.h"

EO_object* eval_array_length(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)result, 0, ((array*)get_parent(obj))->x_length);
    return result;
}