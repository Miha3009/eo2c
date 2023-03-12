EO_object* eval_bytes_concat(EO_object* obj) {
    StackPos pos = stack_store();
    bytes_value left_value = ((bytes*)get_parent(obj))->value;
    unsigned char* left_data = get_bytes_data(get_parent(obj));
    EO_object* right = evaluate(apply_offset(obj, ((bytes_concat*)obj)->b));
    bytes_value right_value = ((bytes*)right)->value;
    unsigned char* right_data = get_bytes_data(right);
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value new_value;
    new_value.length = left_value.length + right_value.length;
    unsigned char* new_data = (unsigned char*)stack_alloc(new_value.length);
    std::memcpy(new_data, left_data, left_value.length);
    std::memcpy(new_data + left_value.length, right_data, right_value.length);
    init_bytes((bytes*)result, 0, new_value);
	stack_restore(pos);
    return move_object(result);
}