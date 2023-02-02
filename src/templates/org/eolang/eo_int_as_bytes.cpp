#include "bytes.h"
#include <algorithm>

EO_object* eval_EO_int_as_bytes(EO_object* obj) {
    long long value = ((EO_int*)get_parent(obj))->value;
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value new_value;
    new_value.length = 8;
    new_value.data = (unsigned char*)stack_alloc(8);
    std::reverse_copy((char*)&value, (char*)&value + 8, new_value.data);
    init_bytes((bytes*)result, 0, new_value);
    return result;
}