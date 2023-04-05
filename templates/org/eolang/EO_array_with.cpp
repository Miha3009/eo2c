EO_object* eval_EO_array_with(EO_object* obj) {
    EO_object* parent = get_parent(obj);
    int len = get_array_length(parent);
    EO_object* result = make_array(len + 1);
    for(int i = 0; i < len; ++i) {
        add_attribute_to_array(result, clone(array_at(parent, i)));
    }
    add_attribute_to_array(result, clone(evaluate(apply_offset(obj, ((EO_array_with*)obj)->x))));
    return result;
}