EO_object* eval_bytes_EO_not(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value value = ((bytes*)get_parent(obj))->value;
    bytes_value new_value;
    new_value.length = value.length;
    new_value.data = (unsigned char*)stack_alloc(value.length);
    for(int i = 0; i < value.length; ++i) {
        new_value.data[i] = ~value.data[i];
    }
    init_bytes((bytes*)result, 0, new_value);
    return result;
}