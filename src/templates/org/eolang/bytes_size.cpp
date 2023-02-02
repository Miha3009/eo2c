#include "int.h"

EO_object* eval_bytes_size(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)result, 0, ((bytes*)get_parent(obj))->value.length);
    return result;
}