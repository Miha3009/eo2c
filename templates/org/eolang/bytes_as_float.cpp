#include "float.h"
#include <algorithm>

EO_object* eval_bytes_as_float(EO_object* obj) {
    bytes_value value = ((bytes*)get_parent(obj))->value;
    if(value.length != 8) {
        throw std::invalid_argument("incorrect bytes length for float. Expected 8, actual " + std::to_string(value.length));        
    }
    double new_value;
    unsigned char* data = get_bytes_data(get_parent(obj));
    std::reverse_copy(data, data + 8, (char*)&new_value);
    EO_object* result = stack_alloc(sizeof(EO_float));
    init_EO_float((EO_float*)result, 0, new_value);
    return result;
}