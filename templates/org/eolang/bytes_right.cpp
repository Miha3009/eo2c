#include "int.h"

EO_object* eval_bytes_right(EO_object* obj) {
 	StackPos pos = stack_store();
    int shift = (int)((EO_int*)evaluate(apply_offset(obj, ((bytes_right*)obj)->x)))->value;
    bool left = shift < 0;
    if(left) {
        shift = -shift;
    }
    int bitShift = shift & 7;
    int byteShift = shift >> 3;
    unsigned char preBuffer = 0;
    unsigned char postBuffer = 0;
    bytes_value value = ((bytes*)get_parent(obj))->value;
    if(byteShift > value.length) {
        byteShift = value.length;
    }
	stack_restore(pos);
    EO_object* result = stack_alloc(sizeof(bytes));
    unsigned char* data = get_bytes_data(get_parent(obj));
    bytes_value new_value;
    new_value.length = value.length;
    unsigned char* new_data = (unsigned char*)stack_alloc(value.length);
    if(left) {
        unsigned char mask = 0xff << (8 - bitShift);
        for(int i = value.length - 1; i >= byteShift; --i) {
            postBuffer = (data[i+byteShift] & mask) >> (8 - bitShift);
            new_data[i] = data[i+byteShift] << bitShift;
            new_data[i] |= preBuffer;
            preBuffer = postBuffer;
        }
        for(int i = byteShift - 1; i >= 0; --i) {
            new_data[i] = 0;
        }
    } else {
        unsigned char mask = 0xff >> (8 - bitShift);
        for(int i = 0; i < byteShift; ++i) {
            new_data[i] = 0;
        }
        for(int i = byteShift; i < value.length; ++i) {
            postBuffer = (data[i-byteShift] & mask) << (8 - bitShift);
            new_data[i] = data[i-byteShift] >> bitShift;
            new_data[i] |= preBuffer;
            preBuffer = postBuffer;
        }
    }
    init_bytes((bytes*)result, 0, new_value);
    return result;
}