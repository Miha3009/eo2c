#include "bool.h"

EO_object* eval_bytes_eq(EO_object* obj) {
    StackPos pos = stack_store();
    bytes* obj1 = (bytes*)get_parent(obj);
    bytes* obj2 = (bytes*)evaluate(apply_offset(obj, ((bytes_eq*)obj)->x));
    bool value;
    if(obj1->value.length != obj2->value.length) {
        value = false;
    } else {
        value = true;
        unsigned char* data1 = get_bytes_data((EO_object*)obj1);
        unsigned char* data2 = get_bytes_data((EO_object*)obj2);
        for(int i = 0; i < obj1->value.length; ++i) {
            if(data1[i] != data2[i]) {
                value = false;
                break;
            }
        }
    }
    stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, value);
    return result;
}