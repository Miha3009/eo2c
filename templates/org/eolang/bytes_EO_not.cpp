EO_object* eval_bytes_EO_not(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value value = ((bytes*)get_parent(obj))->value;
    unsigned char* data = get_bytes_data(get_parent(obj));
    bytes_value new_value;
    new_value.length = value.length;
    unsigned char* new_data = (unsigned char*)stack_alloc(value.length);
    for(int i = 0; i < value.length; ++i) {
        new_data[i] = ~data[i];
    }
    init_bytes((bytes*)result, 0, new_value);
    return result;
}