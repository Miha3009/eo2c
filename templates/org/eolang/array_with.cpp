EO_object* eval_array_with(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(array));
    init_array((array*)result, 0);
    array* arr = (array*)get_parent(obj);
    EO_object* attrs[arr->x_length + 1];
    for(int i = 0; i < arr->x_length; ++i) {
        attrs[i] = clone(arr->x[i]);
    }
    attrs[arr->x_length] = clone(((array_with*)obj)->x);
    add_attributes_(result, attrs, arr->x_length + 1);
    return result;
}