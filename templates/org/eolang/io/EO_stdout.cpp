#include "../string.h"
#include "../bool.h"
#include <iostream>

EO_object* eval_EO_stdout(EO_object* obj) {
	StackPos pos = stack_store();
    EO_object* str = evaluate(apply_offset(obj, ((EO_stdout*)obj)->text));
    wprintf(get_string_data(str));
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, true);
    return result;
}