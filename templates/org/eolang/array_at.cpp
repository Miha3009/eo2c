#include "int.h"

EO_object* eval_array_at(EO_object* obj) {
    int i = ((EO_int*)((array_at*)obj)->i)->value;
    return clone(((array*)get_parent(obj))->x[i]);
}