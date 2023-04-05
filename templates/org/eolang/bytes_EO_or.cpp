EO_object* eval_bytes_EO_or(EO_object* obj) {
	StackPos pos = stack_store();
    EO_object* arr = evaluate(apply_offset(obj, ((bytes_EO_or*)obj)->b));
    int arr_len = get_array_length(arr);
    EO_object* operands[arr_len];
    for(int i = 0; i < arr_len; ++i) {
        operands[i] = evaluate(array_at(arr, i));
    }
    int length = ((bytes*)get_parent(obj))->value.length;
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value new_value;
    new_value.length = length;
    unsigned char* new_data = (unsigned char*)stack_alloc(length);
    std::memcpy(new_data, get_bytes_data(get_parent(obj)), length);
    for(int i = 0; i < arr_len; ++i) {
        unsigned char* operand_data = get_bytes_data(operands[i]);
        for(int j = 0; j < length; ++j) {
            new_data[j] |= operand_data[j];
        }
    }
    init_bytes((bytes*)result, 0, new_value);
	stack_restore(pos);
    return move_object(result);
}