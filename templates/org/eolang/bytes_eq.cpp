#include "bool.h"

EO_object* eval_bytes_eq(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(EO_bool));
    bytes* obj1 = (bytes*)get_parent(obj);
    bytes* obj2 = (bytes*)evaluate(apply_offset(obj, ((bytes_eq*)obj)->x));
    bool value;
    if(obj1->value.length != obj2->value.length) {
        value = false;
    } else {
        value = true;
        for(int i = 0; i < obj1->value.length; ++i) {
            if(obj1->value.data[i] != obj2->value.data[i]) {
                value = false;
                break;
            }
        }
    }
    init_EO_bool((EO_bool*)result, 0, value);
    return result;
}