EO_object* eval_EO_int_minus(EO_object* obj) {
    long long value = ((EO_int*)get_parent(obj))->value;
    EO_int_plus* obj_ = (EO_int_plus*)obj;
    for(int i = 0; i < obj_->x_length; ++i) {
        value -= ((EO_int*)(*(obj_->x + i)))->value;
    }
    EO_object* result = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)result, 0, value);
    return result;
}