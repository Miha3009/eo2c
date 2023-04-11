EO_object* eval_list_mappedi(EO_object* obj) {
    EO_object* parent = get_parent(obj);
    EO_object* arr = evaluate(apply_offset(parent, ((list*)parent)->arr));
    EO_object* f = apply_offset(obj, ((list_mappedi*)obj)->f);
    int len = get_array_length(arr);
	EO_object* new_arr[len];
    for(int i = 0; i < len; ++i) {
        EO_object* f_cloned = clone(f);
        add_attribute(f_cloned, clone(array_at(arr, i)), 2);
        EO_object* idx = stack_alloc(sizeof(EO_int));
        init_EO_int((EO_int*)idx, 0, i);
        add_attribute(f_cloned, idx, 2);
        f_cloned = evaluate(f_cloned);
        new_arr[i] = f_cloned;
    }
    EO_object* result = make_array(len);
    for(int i = 0; i < len; ++i) {
		add_attribute_to_array(result, clone(new_arr[i]));
	}
	add_attribute_end(result);
    return result;
}