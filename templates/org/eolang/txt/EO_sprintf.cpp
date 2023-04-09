#include "../int.h"
#include "../float.h"
#include "../string.h"
#include <cwchar>

EO_object* eval_EO_sprintf(EO_object* obj) {
    static wchar_t buf[1 << 12];
    static wchar_t buf2[50];
	StackPos pos = stack_store();
    wchar_t* format = get_string_data(evaluate(apply_offset(obj, ((EO_sprintf*)obj)->format)));
    EO_object* args = evaluate(apply_offset(obj, ((EO_sprintf*)obj)->args));
    int i = 0, j = 0, k = 0;
    while(format[i]) {
        if(format[i] == '%') {
            int s = 0;
            while(format[i] && format[i] != 'd' && format[i] != 'f' && format[i] != 's') {
                buf2[s] = format[i];
                ++s;
                ++i;
            }
            buf2[s] = format[i];
            buf2[s+1] = '\0';
            if(format[i] == 'd') {
                long long value = ((EO_int*)evaluate(array_at(args, k++)))->value;
                j += swprintf(buf + j, buf2, value);
            } else if(format[i] == 'f') {
                double value = ((EO_float*)evaluate(array_at(args, k++)))->value;
                j += swprintf(buf + j, buf2, value);
            } else if (format[i] == 's') {
                EO_object* str = evaluate(array_at(args, k++));
                wchar_t* value = get_string_data(str);
                int len = ((EO_string*)str)->value.length - 1;
                std::memcpy(&buf + j, value, sizeof(wchar_t)*len);
                j += len;
            }
        } else {
            buf[j] = format[i];
            ++j;
        }
        ++i;
    }
    buf[j] = '\0';
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(EO_string));
    string_value new_value;
    new_value.length = j + 1;
    wchar_t* new_data = (wchar_t*)stack_alloc(sizeof(wchar_t)*new_value.length);
    std::memcpy(new_data, &buf, sizeof(wchar_t)*new_value.length);
    init_EO_string((EO_string*)result, 0, new_value);
    return result;
}