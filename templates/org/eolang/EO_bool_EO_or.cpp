EO_object* eval_EO_bool_EO_or(EO_object* obj) {
	StackPos pos = stack_store();
    bool value = ((EO_bool*)get_parent(obj))->value;
    EO_object* arr = evaluate(apply_offset(obj, ((EO_bool_EO_or*)obj)->x));
    for(int i = 0; i < get_array_length(arr); ++i) {
        value = value || ((EO_bool*)(evaluate(array_at(arr, i))))->value;
    }
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, value);
    return result;
}