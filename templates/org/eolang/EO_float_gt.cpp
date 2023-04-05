#include "bool.h"

EO_object* eval_EO_float_gt(EO_object* obj) {
	StackPos pos = stack_store();
    double float1 = ((EO_float*)get_parent(obj))->value;
    double float2 = ((EO_float*)evaluate(apply_offset(obj, ((EO_float_gt*)obj)->x)))->value;
    bool value = float1 > float2;
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_float));
    init_EO_bool((EO_bool*)result, 0, value);
    return (EO_object*)result;
}