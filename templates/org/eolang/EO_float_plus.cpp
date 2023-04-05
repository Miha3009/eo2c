EO_object* eval_EO_float_plus(EO_object* obj) {
	StackPos pos = stack_store();
    double value = ((EO_float*)get_parent(obj))->value;
    EO_object* arr = evaluate(apply_offset(obj, ((EO_float_plus*)obj)->x));
    for(int i = 0; i < get_array_length(arr); ++i) {
        value += ((EO_float*)(evaluate(array_at(arr, i))))->value;
    }
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_float));
    init_EO_float((EO_float*)result, 0, value);
    return result;
}