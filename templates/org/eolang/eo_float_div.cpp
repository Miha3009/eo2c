EO_object* eval_EO_float_div(EO_object* obj) {
    double value = ((EO_float*)get_parent(obj))->value;
    EO_float_div* obj_ = (EO_float_div*)obj;
    for(int i = 0; i < obj_->x_length; ++i) {
        value /= ((EO_float*)(*(obj_->x + i)))->value;
    }
    EO_object* result = stack_alloc(sizeof(EO_float));
    init_EO_float((EO_float*)result, 0, value);
    return result;
}