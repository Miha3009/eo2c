#include "../string.h"

EO_object* eval_stdout(EO_object* obj) {
    std::cout << ((EO_string*)((stdout*)obj->text))->value;
    return nullptr;
}