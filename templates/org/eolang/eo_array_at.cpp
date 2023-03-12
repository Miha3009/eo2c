#include "int.h"

EO_object* eval_EO_array_at(EO_object* obj) {
	StackPos pos = stack_store();
    int i = (int)((EO_int*)evaluate(apply_offset(obj, ((EO_array_at*)obj)->i)))->value;
	stack_restore(pos);
    return clone(array_at(get_parent(obj), i));
}