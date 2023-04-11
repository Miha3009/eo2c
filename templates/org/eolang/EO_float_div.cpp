#include "math/nan.h"
#include "math/negative-infinity.h"
#include "math/positive-infinity.h"
#include <cmath>

EO_object* eval_EO_float_div(EO_object* obj) {
	StackPos pos = stack_store();
    double value = ((EO_float*)get_parent(obj))->value;
    EO_object* arr = evaluate(apply_offset(obj, ((EO_float_div*)obj)->x));
    for(int i = 0; i < get_array_length(arr); ++i) {
        value /= ((EO_float*)(evaluate(array_at(arr, i))))->value;
    }
	stack_restore(pos);
    EO_object* result;
    if(std::isnan(value)) {
        result = stack_alloc(sizeof(EO_nan));
        init_EO_nan((EO_nan*)result, 0);
    } else if(std::isinf(value)) {
        if(value > 0) {
            result = stack_alloc(sizeof(positive_infinity));
            init_positive_infinity((positive_infinity*)result, 0);
        } else {
            result = stack_alloc(sizeof(negative_infinity));
            init_negative_infinity((negative_infinity*)result, 0);
        }
    } else {
        result = stack_alloc(sizeof(EO_float));
        init_EO_float((EO_float*)result, 0, value);
    }
    return result;
}