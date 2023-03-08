#include "../string.h"
#include "../bool.h"
#include <iostream>

EO_object* eval_EO_stdout(EO_object* obj) {
    EO_string* str = (EO_string*)evaluate(apply_offset(obj, ((EO_stdout*)obj)->text));
    std::cout << str->value.data;
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, true);
    return result;
}