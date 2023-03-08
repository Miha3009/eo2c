EO_object* eval_EO_int_plus(EO_object* obj) {
    long long value = ((EO_int*)get_parent(obj))->value;
    EO_object* arr = apply_offset(obj, ((EO_int_plus*)obj)->x);
    for(int i = 0; i < get_array_length(arr); ++i) {
        value += ((EO_int*)(evaluate(array_at(arr, i))))->value;
    }
    EO_object* result = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)result, 0, value);
    return result;
}