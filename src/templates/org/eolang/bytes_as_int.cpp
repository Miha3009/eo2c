#include "int.h"
#include <algorithm>

EO_object* eval_bytes_as_int(EO_object* obj) {
    bytes_value value = ((bytes*)get_parent(obj))->value;
    if(value.length != 8) {
        throw std::invalid_argument("incorrect bytes length for int. Expected 8, actual " + std::to_string(value.length));        
    }
    long long new_value;
    std::reverse_copy(value.data, value.data + 8, (char*)&new_value);
    EO_object* result = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)result, 0, new_value);
    return result;
}