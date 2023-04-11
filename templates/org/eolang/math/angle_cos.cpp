#include "../float.h"
#include <cmath>

EO_object* eval_angle_cos(EO_object* obj) {
	StackPos pos = stack_store();
    double value = ((EO_float*)evaluate(get_parent(obj)))->value;
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_float));
    init_EO_float((EO_float*)result, 0, std::cos(value));
    return result;
}