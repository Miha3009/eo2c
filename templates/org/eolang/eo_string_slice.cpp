EO_object* eval_EO_string_slice(EO_object* obj) {
    std::string value = ((EO_string*)get_parent(obj))->value;
    int start = ((EO_int*)(((EO_string_slice*)obj)->start))->value;
    int len = ((EO_int*)(((EO_string_slice*)obj)->len))->value;
    std::string new_value = value.substr(start, len);
    EO_object* result = stack_alloc(sizeof(EO_string));
    init_EO_string((EO_string*)result, 0, new_value);
    return result;
}