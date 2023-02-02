EO_object* eval_EO_bool_EO_or(EO_object* obj) {
    bool value = ((EO_bool*)get_parent(obj))->value;
    EO_bool_EO_or* obj_ = (EO_bool_EO_or*)obj;
    for(int i = 0; i < obj_->x_length; ++i) {
        value = value || ((EO_bool*)(*(obj_->x + i)))->value;
    }
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, value);
    return result;
}