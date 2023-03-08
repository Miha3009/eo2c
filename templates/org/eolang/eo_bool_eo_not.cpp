EO_object* eval_EO_bool_EO_not(EO_object* obj) {
    EO_object* var_0 = getParent(obj);
    EO_object* var_1 = getSub(var_0, 1);
    EO_object* var_2 = stack_alloc(size_of(EO_bool));
    init_EO_bool((EO_bool*)var_2, NULL, false);
    add_attributes(var_1, var_2);
    EO_object* result = var_1->head.eval();
    reset_attributes(var_1);
    bool value = ((EO_bool*)get_parent(obj))->value;
    EO_bool_EO_or* obj_ = (EO_bool_EO_or*)obj;
    for(int i = 0; i < obj_->x_length; ++i) {
        value = value || ((EO_bool*)(*(obj_->x + i)))->value;
    }
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, value);
    return result;
}