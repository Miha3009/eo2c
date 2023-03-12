#include "int.h"

EO_object* eval_bytes_slice(EO_object* obj) {
	StackPos pos = stack_store();
    bytes_slice* obj_ = (bytes_slice*)obj;
    long long len = ((EO_int*)evaluate(apply_offset(obj, obj_->len)))->value;
    long long start = ((EO_int*)evaluate(apply_offset(obj, obj_->start)))->value;
    bytes_value new_value;
    new_value.length = len;
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(bytes));
    unsigned char* data = (unsigned char*)stack_alloc(len);
    std::memcpy(data, get_bytes_data(get_parent(obj)) + start, len);
    init_bytes((bytes*)result, 0, new_value);
    return result;
}