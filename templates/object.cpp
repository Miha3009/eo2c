#include "object.h"
#include "org/eolang/int.h"
#include "org/eolang/float.h"
#include "org/eolang/bool.h"
#include "org/eolang/string.h"
#include "org/eolang/array.h"
#include "org/eolang/bytes.h"

Stack stack;
int obj_count = 0;

EO_object* evaluate(EO_object* obj) {
  return obj->head.eval(obj);
}

EO_object* get_sub(EO_object* obj, Tag tag, bool need_copy) {
  const std::unordered_map<Tag, int>* offset_table = obj->head.offset_map;
  auto result_it = offset_table->find(tag);
  if(result_it == offset_table->end()) {
    EO_object* newObj = evaluate(obj);
    while(newObj != obj) {
      obj = newObj;
      offset_table = obj->head.offset_map;
      result_it = offset_table->find(tag);
      if(result_it != offset_table->end()) {
        break;
      }
      newObj = evaluate(obj);
    }
    if(result_it == offset_table->end()) {
      wprintf(L"Error: tag %d not found\n", tag);
      print_object(obj);
      exit(0);
    }
  }
  EO_object* result = (EO_object*)((StackPos)obj + (*result_it).second);
  StackPos last_attribute = (StackPos)((int*)(obj + 1) + obj->head.attr_count);
  if(last_attribute - (StackPos)result > 0) {
    int offset = *((int*)result);
    if(!offset) {
      wprintf(L"Error: tag %d not initialized\n", tag);
      print_object(obj);
      exit(0);
    }
    result = apply_offset(obj, offset);
  }
  if(need_copy) {
    result = clone(result);
  }
  return result;
}

EO_object* get_parent(EO_object* obj) {
  if(!obj->head.parent_offset) {
    wprintf(L"Error: parent not found\n");
    print_object(obj);
    exit(0);
  }
  return (EO_object*)((StackPos)obj - obj->head.parent_offset);
}

EO_object* get_home(EO_object* obj) {
  return obj->head.home;
}

EO_object* get_id(EO_object* obj) {
  EO_object* result = stack_alloc(sizeof(EO_int));
  init_EO_int((EO_int*)result, 0, obj->head.id);
  return result;
}

void init_head(EO_object* obj,
               EO_object* (*eval)(EO_object*),
               int parent_offset,
               int varargs_pos,
               size_t size,
               const std::unordered_map<Tag, int>* offset_map) {
  obj->head.id = obj_count++;
  obj->head.eval = eval;
  obj->head.parent_offset = parent_offset;
  obj->head.home = (EO_object*)((StackPos)obj - parent_offset);
  obj->head.attr_count = 0;
  obj->head.varargs_pos = varargs_pos;
  obj->head.size = size;
  obj->head.offset_map = offset_map;
}

EO_object* stack_alloc(size_t size) {
  EO_object* obj = (EO_object*)stack.pos;
  stack.pos += size;
  if(stack.pos > stack.end) {
    wprintf(L"Error: stack overflow\n");
    exit(0);
  }
  return obj;
}

void add_attribute_start(EO_object* obj, int length) {
  if(obj->head.varargs_pos == 0) {
      EO_object* arr = make_array(length - obj->head.attr_count);
      *((int*)(obj + 1) + obj->head.varargs_pos) = calc_offset(obj, arr);
      ++obj->head.attr_count;
  }
}

void add_attribute_end(EO_object* obj) {
  obj->head.size = stack.pos - (StackPos)obj;
}

void add_attribute(EO_object* obj, EO_object* attr, int length) {
  if(obj->head.attr_count >= (unsigned int)obj->head.varargs_pos) {
    int array_offset = *((int*)(obj + 1) + obj->head.varargs_pos);
    EO_object* arr = apply_offset(obj, array_offset);
    *((int*)((EO_array*)arr + 1) + arr->head.attr_count) = calc_offset(arr, attr);
    ++arr->head.attr_count;
    arr->head.size += attr->head.size;
  } else {
    *((int*)(obj + 1) + obj->head.attr_count) = calc_offset(obj, attr);
    ++obj->head.attr_count;
    if(obj->head.attr_count == obj->head.varargs_pos) {
      EO_object* arr = make_array(length - obj->head.attr_count);
      *((int*)(obj + 1) + obj->head.varargs_pos) = calc_offset(obj, arr);
      ++obj->head.attr_count;
    }
  }
}

void add_attribute_by_tag(EO_object* obj, EO_object* attr, Tag tag) {
  const std::unordered_map<Tag, int>* offset_table = obj->head.offset_map;
  auto result_it = offset_table->find(tag);
  if(result_it == offset_table->end()) {
    wprintf(L"Error: tag %d not found\n", tag);
    exit(0);
  }
  int* result = (int*)((StackPos)obj + (*result_it).second);
  *result = calc_offset(obj, attr);
  ++obj->head.attr_count;
}

void add_array_attribute(EO_object* obj, EO_object* arr) {
  *((int*)(obj + 1) + obj->head.varargs_pos) = calc_offset(obj, arr);
  ++obj->head.attr_count;
}

void add_attribute_to_array(EO_object* obj, EO_object* attr) {
  *((int*)((EO_array*)obj + 1) + obj->head.attr_count) = calc_offset(obj, attr);
  ++obj->head.attr_count;
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
  std::memmove(obj_clone, obj, obj->head.size);
  obj_clone->head.parent_offset = 0;
  return obj_clone;
}

unsigned char* get_bytes_data(EO_object* obj) {
  return (unsigned char*)((StackPos)obj + sizeof(bytes));
}

string_value make_string(const wchar_t* str) {
  string_value result;
  int i = 0;
  while(str[i] != L'\0') ++i;
  result.length = i + 1;
  wchar_t* data = (wchar_t*)stack_alloc(sizeof(wchar_t) * result.length);
  for(int i = 0; i < result.length; ++i) {
    data[i] = str[i];
  }
  return result;
}

wchar_t* get_string_data(EO_object* obj) {
  return (wchar_t*)((StackPos)obj + sizeof(EO_string));
}

long long get_int_data(EO_object* obj) {
  return ((EO_int*)obj)->value;
}

double get_float_data(EO_object* obj) {
  return ((EO_float*)obj)->value;
}

bool get_bool_data(EO_object* obj) {
  return ((EO_bool*)obj)->value;
}

EO_object* make_inner_application(EO_object* parent, EO_object* (*eval)(EO_object*)) {
  EO_object* inner_application = stack_alloc(sizeof(EO_object));
  init_head(inner_application, eval, calc_offset(parent, inner_application), -1, sizeof(EO_object), &offset_default);
  return inner_application;
}

EO_object* run_inner_application(EO_object* obj, EO_object* (*eval)(EO_object*)) {
  static char buf[sizeof(EO_object)];
  EO_object* tmp = (EO_object*)buf;
  tmp->head.home = obj;
  return eval(tmp);
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
  int i = 0;
  if(str[0] == '-') {
    ++i;
  }
  for(; str[i] != '\0'; ++i) {
    if(str[i] < '0' || str[i] > '9') {
      return false;
    }
  }
  return true;
}

bool is_float(char* str) {
  int i = 0;
  if(str[0] == '-') {
    ++i;
  }
  bool dot = false;
  for(; str[i] != '\0'; ++i) {
    if(str[i] == '.') {
      if(dot) {
        return false;
      }
      dot = true;
      continue;
    }
    if(str[i] < '0' || str[i] > '9') {
      return false;
    }
  }
  return dot;
}

EO_object* make_object_from_arg(char* arg) {
  if(is_int(arg)) {
    EO_object* obj = stack_alloc(sizeof(EO_int));
    init_EO_int((EO_int*)obj, 0, std::atoll(arg));
    return obj;
  } else if(is_float(arg)) {
    EO_object* obj = stack_alloc(sizeof(EO_float));
    init_EO_float((EO_float*)obj, 0, std::atof(arg));
    return obj;
  } else {
    EO_object* obj = stack_alloc(sizeof(EO_string));
    const size_t len = strlen(arg) + 1;
    wchar_t* warg = new wchar_t[len];
    mbstowcs(warg, arg, len);
    init_EO_string((EO_string*)obj, 0, make_string(warg));
    delete warg;
    return obj;
  }
  return nullptr;
}

void print_object(EO_object* obj) {
  wprintf(L"Object id = %d\n", obj->head.id);
}
