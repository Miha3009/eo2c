#include "../int.h"
#include "../float.h"
#include "../string.h"
#include <cwchar>

EO_object* eval_EO_sscanf(EO_object* obj) {
    static wchar_t buf[1 << 12];
	StackPos pos = stack_store();
    wchar_t* format = get_string_data(evaluate(apply_offset(obj, ((EO_sscanf*)obj)->format)));
    wchar_t* read = get_string_data(evaluate(apply_offset(obj, ((EO_sscanf*)obj)->read)));
    int j = 0;
    for(int i = 0; format[i]; ++i) {
        if(format[i] == '%') {
            ++j;
        }
        ++i;
    }
    EO_object* result = make_array(j);
    j = 0;
    for(int i = 0; format[i]; ++i) {
        if(format[i] == '%') {
            int s = 0;
            ++i;
            if(format[i] == 'd') {
                long long value;
                swscanf(read + j, L"%lld%n", &value, &s);
                j += s;
                EO_object* element = stack_alloc(sizeof(EO_int));
                init_EO_int((EO_int*)element, 0, value);
                add_attribute_to_array(result, element);
            } else if(format[i] == 'f') {
                double value;
                swscanf(read + j, L"%lf%n", &value, &s);
                j += s;
                EO_object* element = stack_alloc(sizeof(EO_float));
                init_EO_float((EO_float*)element, 0, value);
                add_attribute_to_array(result, element);
            } else if (format[i] == 's') {
                swscanf(read + j, L"%ls%n", buf, &s);
                j += s;
                string_value value;
                value.length = s + 1;
                EO_object* element = stack_alloc(sizeof(EO_string));
                init_EO_string((EO_string*)element, 0, value);
                wchar_t* data = (wchar_t*)stack_alloc(sizeof(wchar_t)*value.length);
                data[value.length-1] = '\0';
                std::memcpy(data, buf, sizeof(wchar_t)*value.length);
                add_attribute_to_array(result, element);
            }
        } else {
            ++j;
        }
    }
    add_attribute_end(result);
    stack_restore(pos);
    return move_object(result);
}