#include "string.h"
#include <stdexcept>
#include <cstdlib>

EO_object* eval_error(EO_object* obj) {
    static char buf[1 << 12];
	StackPos pos = stack_store();
    EO_object* message = evaluate(apply_offset(obj, ((error*)obj)->message));
    wcstombs(buf, get_string_data(message), ((EO_string*)message)->value.length);
    stack_restore(pos);
    throw std::runtime_error(buf);
}