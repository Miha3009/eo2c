#include "int.h"

EO_object* eval_bytes_slice(EO_object* obj) {
    bytes_slice* obj_ = (bytes_slice*)obj;
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value value = ((bytes*)get_parent(obj))->value;
    bytes_value new_value;
    int len = ((EO_int*)obj_->len)->value;
    int start = ((EO_int*)obj_->start)->value;
    new_value.length = len;
    new_value.data = (unsigned char*)stack_alloc(len);
    std::memcpy(new_value.data, value.data + start, len);
    init_bytes((bytes*)result, 0, new_value);
    return result;
}