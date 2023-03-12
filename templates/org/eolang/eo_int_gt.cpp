#include "bool.h"

EO_object* eval_EO_int_gt(EO_object* obj) {
	StackPos pos = stack_store();
    long long int1 = ((EO_int*)get_parent(obj))->value;
    long long int2 = ((EO_int*)evaluate(apply_offset(obj, ((EO_int_gt*)obj)->x)))->value;
    bool value = int1 > int2;
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, value);
    return (EO_object*)result;
}