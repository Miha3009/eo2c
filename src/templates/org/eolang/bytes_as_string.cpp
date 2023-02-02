#include "string.h"

EO_object* eval_bytes_as_string(EO_object* obj) {
    bytes_value value = ((bytes*)get_parent(obj))->value;
    std::string new_value;
    new_value.append((char*)value.data, value.length);
    EO_object* result = stack_alloc(sizeof(EO_string));
    init_EO_string((EO_string*)result, 0, new_value);
    return result;
}