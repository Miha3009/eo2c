EO_object* eval_EO_bool_EO_if(EO_object* obj) {
    if(((EO_bool*)get_parent(obj))->value) {
        return evaluate(((EO_bool_EO_if*)obj)->t);
    } else {
        return evaluate(((EO_bool_EO_if*)obj)->f);
    }
}