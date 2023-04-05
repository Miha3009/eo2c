EO_object* eval_EO_string_slice(EO_object* obj) {
	StackPos pos = stack_store();
    EO_string_slice* obj_ = (EO_string_slice*)obj;
    long long len = ((EO_int*)evaluate(apply_offset(obj, obj_->len)))->value;
    long long start = ((EO_int*)evaluate(apply_offset(obj, obj_->start)))->value;
    wchar_t* data = get_string_data(get_parent(obj));
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_string));
    string_value new_value;
    new_value.length = len + 1;
    wchar_t* new_data = (wchar_t*)stack_alloc(sizeof(wchar_t)*new_value.length);
    std::memcpy(new_data, data + start, sizeof(wchar_t)*len);
    new_data[len] = L'\0';
    init_EO_string((EO_string*)result, 0, new_value);
    return result;
}