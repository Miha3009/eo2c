#include "string.h"

EO_object* eval_bytes_as_string(EO_object* obj) {
    int len = ((bytes*)get_parent(obj))->value.length;
    unsigned char* data = get_bytes_data(get_parent(obj));
    EO_object* result = stack_alloc(sizeof(EO_string));
    string_value new_value;
    new_value.length = len / sizeof(wchar_t);
    unsigned char* new_data = (unsigned char*)stack_alloc(len);
    std::memcpy(new_data, data, len);
    init_EO_string((EO_string*)result, 0, new_value);
    return result;
}