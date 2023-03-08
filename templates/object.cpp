#include "object.h"
#include "org/eolang/int.h"
#include "org/eolang/string.h"
#include "org/eolang/array.h"

Stack stack;
int obj_count = 0;

EO_object* get_parent(EO_object* obj) {
  return (EO_object*)((StackPos)obj - obj->head.parent_offset);
}

EO_object* get_sub(EO_object* obj, Tag tag, bool need_copy) {
  //std::cout << "GET_SUB: " << (StackPos)obj - stack.data << " " << tag << " " << ((StackPos)obj + obj->head.offset_table->at(tag)) - stack.data << std::endl;
  if(obj->head.ref >= stack.data && obj->head.ref <= stack.data + STACK_SIZE) {
    obj = evaluate(obj);
  }
  const std::unordered_map<Tag, int>* offset_table = (const std::unordered_map<Tag, int>*)obj->head.ref;
  auto result_it = offset_table->find(tag);
  if(result_it == offset_table->end()) {
    if(!need_copy) {
      obj = evaluate(obj);
      offset_table = ((const std::unordered_map<Tag, int>*)obj->head.ref);
      result_it = offset_table->find(tag);
    }
    if(result_it == offset_table->end()) {
      std::cout << "ERROR TAG " << tag << " NOT FOUND\n";
      exit(0);
    }
  }
  EO_object* result = (EO_object*)((StackPos)obj + (*result_it).second);
  EO_object* last_attribute = (EO_object*)((EO_object**)(obj + 1) + obj->head.attr_count);
  if(((void*)result) < last_attribute) {
    result = apply_offset(obj, *((int*)result));
  }
  if(need_copy) {
    result = clone(result);
  }
  return result;
}

EO_object* ensure_sub(EO_object* obj, Tag tag) {
  if(obj->head.ref >= stack.data && obj->head.ref <= stack.data + STACK_SIZE) {
    obj = evaluate(obj);
  }
  const std::unordered_map<Tag, int>* offset_table = (const std::unordered_map<Tag, int>*)obj->head.ref;
  auto result_it = offset_table->find(tag);
  if(result_it == offset_table->end()) {
    obj = evaluate(obj);
    offset_table = (const std::unordered_map<Tag, int>*)obj->head.ref;
    result_it = offset_table->find(tag);
    if(result_it == offset_table->end()) {
      std::cout << "ERROR TAG " << tag << " NOT FOUND\n";
      exit(0);
    }
  }
  return obj;
}

EO_object* evaluate(EO_object* obj) {
//  std::cout << "EVAL " << (char*)obj - stack.data << " " << obj->head.id << std::endl;
  return obj->head.eval(obj);
}

void init_head(EO_object* obj,
               EO_object* (*eval)(EO_object*),
               int parent_offset,
               int varargs_pos,
               size_t size,
               const void* ref) {
  obj->head.id = obj_count++;
  obj->head.eval = eval;
  obj->head.parent_offset = parent_offset;
  obj->head.attr_count = 0;
  obj->head.varargs_pos = varargs_pos;
  obj->head.size = size;
  obj->head.ref = ref;
//  std::cout << "OBJ " << (char*)obj - stack.data << " " << obj->head.id << std::endl;
}

void add_attribute(EO_object* obj, EO_object* attr, int length) {
  if(obj->head.attr_count == obj->head.varargs_pos) {
    int array_offset = *((int*)(obj + 1) + obj->head.varargs_pos);
    EO_object* arr;
    if(array_offset == 0) {
      arr = make_array(length - obj->head.attr_count);
      *((int*)(obj + 1) + obj->head.attr_count) = calc_offset(obj, arr);
    } else {
      arr = apply_offset(obj, array_offset);
    }
    *((int*)((EO_array*)arr + 1) + arr->head.attr_count) = calc_offset(arr, attr);
    //attr->head.parent_offset = calc_offset(arr, attr);
    ++arr->head.attr_count;
    arr->head.size += attr->head.size;
  } else {
    *((int*)(obj + 1) + obj->head.attr_count) = calc_offset(obj, attr);
    //attr->head.parent_offset = calc_offset(obj, attr);
    ++obj->head.attr_count;
  }
}

void add_array_attribute(EO_object* obj, EO_object* arr) {
  *((int*)(obj + 1) + obj->head.varargs_pos) = calc_offset(obj, arr);
  ++obj->head.attr_count;
  //arr->head.parent_offset = calc_offset(obj, arr);
}

void add_attribute_to_array(EO_object* obj, EO_object* attr) {
  *((int*)((EO_array*)obj + 1) + obj->head.attr_count) = calc_offset(obj, attr);
  ++obj->head.attr_count;
  //attr->head.parent_offset = calc_offset(obj, attr);
}

EO_object* clone(EO_object* obj) {
  EO_object* obj_clone = stack_alloc(obj->head.size);
  std::memcpy(obj_clone, obj, obj->head.size);
  if(obj_clone->head.parent_offset != 0) {
    obj_clone->head.parent_offset += calc_offset(obj, obj_clone);
  }
  return obj_clone;
}

EO_object* move_object(EO_object* obj) {
  EO_object* obj_clone = stack_alloc(obj->head.size);
  if(obj_clone == obj) {
    return obj;
  }
  if((StackPos)obj + obj->head.size > (StackPos)obj_clone) {
    std::memmove(obj_clone, obj, obj->head.size);
  } else {
    std::memcpy(obj_clone, obj, obj->head.size);
  }
  if(obj_clone->head.parent_offset != 0) {
    obj_clone->head.parent_offset += calc_offset(obj, obj_clone);
  }
  return obj_clone;
}

string_value make_string(const char* str) {
  string_value result;
  int i = 0;
  while(str[i] != '\0') ++i;
  result.length = i + 1;
  result.data = (char*)stack_alloc(sizeof(char) * result.length);
  for(int i = 0; i < result.length; ++i) {
    result.data[i] = str[i];
  }
  return result;
}

EO_object* make_meta_object(EO_object* parent, EO_object* (*eval)(EO_object*)) {
  EO_object* meta_obj = stack_alloc(sizeof(EO_object));
  init_head(meta_obj, eval, calc_offset(parent, meta_obj), -1, sizeof(EO_object), parent);
  return meta_obj;
}

EO_object* make_array(int length) {
  EO_object* result = stack_alloc(sizeof(EO_array));
  init_EO_array((EO_array*)result, 0, length);
  stack.pos += length * sizeof(int);
  result->head.size += length * sizeof(int);
  return result;
}

EO_object* array_at(EO_object* arr, int i) {
  return apply_offset(arr, *((int*)((EO_array*)arr + 1) + i));
}

int get_array_length(EO_object* obj) {
  return ((EO_array*)obj)->value;
}

bool is_int(char* str) {
  for(int i = 0; str[i] != '\0'; ++i) {
    if(str[i] < '0' || str[i] > '9') {
      return false;
    }
  }
  return true;
}

bool is_string(char* str) {
  if(str[0] != '"') {
    return false;
  }
  int i = 0;
  for(; str[i] != '\0'; ++i);
  if(i == 0 || str[i-1] != '"') {
    return false;
  }
  str[i-1] = '\0';
  return true;
}

EO_object* make_object_from_arg(char* arg) {
  if(is_int(arg)) {
    EO_object* obj = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)obj, 0, std::atoll(arg));
    return obj;
  } else if(is_string(arg)) {
    EO_object* obj = stack_alloc(sizeof(EO_string));
    init_EO_string((EO_string*)obj, 0, make_string(arg + 1));
    return obj;
  }
  std::cout << "A suitable type for the argument " << arg << " was not found\n";
  return nullptr;
}
