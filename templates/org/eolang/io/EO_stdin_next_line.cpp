#include "../string.h"

EO_object* eval_EO_stdin_next_line(EO_object* obj) {
    static wchar_t buf[1 << 12];
    int i = 0;
    while(wscanf(L"%c", &buf[i])) {
        if(buf[i] == L'\n') {
            break;
        } else {
            ++i;
        }
    }
    buf[i] = '\0';
    EO_object* result = stack_alloc(sizeof(EO_string));
    string_value new_value;
    new_value.length = i + 1;
    init_EO_string((EO_string*)result, 0, new_value);
    wchar_t* new_data = (wchar_t*)stack_alloc(sizeof(wchar_t)*new_value.length);
    std::memcpy(new_data, &buf, sizeof(wchar_t)*new_value.length);
    return result;
}