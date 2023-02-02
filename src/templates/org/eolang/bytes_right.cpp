#include "int.h"

EO_object* eval_bytes_right(EO_object* obj) {
    EO_object* result = stack_alloc(sizeof(bytes));
    bytes_value value = ((bytes*)get_parent(obj))->value;
    bytes_value new_value;
    new_value.length = value.length;
    new_value.data = (unsigned char*)stack_alloc(value.length);
    int shift = (int)((EO_int*)((bytes_right*)obj)->x)->value;
    bool right = true;
    if(shift < 0) {
        shift = -shift;
        right = false;
    }
    int bitShift = shift & 7;
    int byteShift = shift >> 3;
    unsigned char preBuffer = 0;
    unsigned char postBuffer = 0;
    if(byteShift > value.length) {
        byteShift = value.length;
    }
    if(right) {
        unsigned char mask = 0xff >> (8 - bitShift);
        for(int i = 0; i < byteShift; ++i) {
            new_value.data[i] = 0;
        }
        for(int i = byteShift; i < value.length; ++i) {
            postBuffer = (value.data[i-byteShift] & mask) << (8 - bitShift);
            new_value.data[i] = value.data[i-byteShift] >> bitShift;
            new_value.data[i] |= preBuffer;
            preBuffer = postBuffer;
        }
    } else {
        unsigned char mask = 0xff << (8 - bitShift);
        for(int i = value.length - 1; i >= byteShift; --i) {
            postBuffer = (value.data[i+byteShift] & mask) >> (8 - bitShift);
            new_value.data[i] = value.data[i+byteShift] << bitShift;
            new_value.data[i] |= preBuffer;
            preBuffer = postBuffer;
        }
        for(int i = byteShift - 1; i >= 0; --i) {
            new_value.data[i] = 0;
        }
    }
    init_bytes((bytes*)result, 0, new_value);
    return result;
}