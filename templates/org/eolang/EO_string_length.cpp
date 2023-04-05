#include "int.h"

EO_object* eval_EO_string_length(EO_object* obj) {
    long long value = (long long)((EO_string*)get_parent(obj))->value.length - 1;
    EO_object* result = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)result, 0, value);
    return result;
}