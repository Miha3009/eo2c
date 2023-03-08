EO_object* eval_bytes_EO_xor(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value value = ((bytes*)get_parent(obj))->value;
    bytes_value new_value;
    new_value.length = value.length;
    new_value.data = (unsigned char*)stack_alloc(value.length);
    std::memcpy(new_value.data, value.data, value.length);
    bytes_EO_xor* obj_ = (bytes_EO_xor*)obj;
    for(int i = 0; i < obj_->b_length; ++i) {
        bytes* operand = (bytes*)(*(obj_->b + i));
        for(int j = 0; j < value.length; ++j) {
            new_value.data[j] ^= operand->value.data[j];
        }
    }
    init_bytes((bytes*)result, 0, new_value);
    return result;
}