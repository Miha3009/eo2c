#include "bytes.h"

EO_object* eval_EO_string_as_bytes(EO_object* obj) {
    int len = ((EO_string*)get_parent(obj))->value.length;
    wchar_t* data = get_string_data(get_parent(obj));
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value new_value;
    new_value.length = sizeof(wchar_t)*len;
    unsigned char* new_data = (unsigned char*)stack_alloc(new_value.length);
    std::memcpy(new_data, (unsigned char*)data, new_value.length);
    init_bytes((bytes*)result, 0, new_value);
    return result;
}