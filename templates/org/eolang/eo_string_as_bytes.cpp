#include "bytes.h"

EO_object* eval_EO_string_as_bytes(EO_object* obj) {
    std::string value = ((EO_string*)get_parent(obj))->value;
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value new_value;
    new_value.length = value.size();
    new_value.data = (unsigned char*)stack_alloc(value.size());
    for(int i = 0; i < value.size(); ++i) {
        new_value.data[i] = (unsigned char)value[i];
    }
    init_bytes((bytes*)result, 0, new_value);
    return result;
}