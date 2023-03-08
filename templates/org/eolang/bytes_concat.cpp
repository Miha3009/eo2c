EO_object* eval_bytes_concat(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value left_value = ((bytes*)get_parent(obj))->value;
    bytes_value right_value = ((bytes*)(((bytes_concat*)obj)->b))->value;
    bytes_value new_value;
    new_value.length = left_value.length + right_value.length;
    new_value.data = (unsigned char*)stack_alloc(new_value.length);
    std::memcpy(new_value.data, left_value.data, left_value.length);
    std::memcpy(new_value.data + left_value.length, right_value.data, right_value.length);
    init_bytes((bytes*)result, 0, new_value);
    return result;
}