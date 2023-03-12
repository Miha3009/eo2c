#include "bool.h"

EO_object* eval_seq(EO_object* obj) {
	StackPos pos = stack_store();
    EO_object* arr = evaluate(apply_offset(obj, ((seq*)obj)->steps));
    for(int i = 0; i < get_array_length(arr); ++i) {
        evaluate(array_at(arr, i));
    }
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_bool));
    init_EO_bool((EO_bool*)result, 0, true);
    return result;
}