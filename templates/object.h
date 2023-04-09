#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>

static const int STACK_SIZE = 1 << 20;

typedef int Tag;

struct EO_object;

const std::unordered_map<Tag, int> offset_default;

struct EO_head {
  int id;
  EO_object* (*eval)(EO_object*);
  int parent_offset;
  EO_object* home;
  int attr_count;
  int varargs_pos;
  size_t size;
  const std::unordered_map<Tag, int>* offset_map;
};

struct EO_object {
  EO_head  head;
};

EO_object* evaluate(EO_object* obj);

EO_object* get_sub(EO_object* obj, Tag tag, bool need_copy);

EO_object* get_parent(EO_object* obj);

EO_object* get_home(EO_object* obj);

void init_head(EO_object* obj,
               EO_object* (*eval)(EO_object*),
               int parent_offset,
               int varargs_pos,
               size_t size,
               const std::unordered_map<Tag, int>* offset_map);

typedef char* StackPos;

struct Stack {
  char data[STACK_SIZE];
  StackPos pos = (StackPos)&data;
};

extern Stack stack;

inline EO_object* stack_alloc(size_t size) {
  EO_object* obj = (EO_object*)stack.pos;
  stack.pos += size;
  return obj;
}

inline StackPos stack_store() {
  return stack.pos;
}

inline void stack_restore(StackPos pos) {
  stack.pos = pos;
}

inline int calc_offset(EO_object* parent, EO_object* obj) {
  return (int)((StackPos)obj - (StackPos)parent);
}

inline EO_object* apply_offset(EO_object* obj, int offset) {
  return (EO_object*)((StackPos)obj + offset);
}

inline void update_size(EO_object* obj) {
  obj->head.size = stack.pos - (StackPos)obj;
}

void add_attribute(EO_object* obj, EO_object* attr, int length);

void add_array_attribute(EO_object* obj, EO_object* arr);

void add_attribute_to_array(EO_object* obj, EO_object* attr);

EO_object* clone(EO_object* obj);

EO_object* move_object(EO_object* obj);

struct bytes_value {
  int length;
};

template <typename... Bytes>
bytes_value make_bytes(Bytes... bytes) {
  unsigned char bytes_[sizeof...(bytes)] = {bytes...};
  bytes_value result;
  result.length = sizeof...(Bytes);
  unsigned char* data = (unsigned char*)stack_alloc(sizeof...(bytes));
  for(int i = 0; i < sizeof...(bytes); ++i) {
    data[i] = bytes_[i];
  }
  return result;
}

unsigned char* get_bytes_data(EO_object* obj);

struct string_value {
  int length;
};

string_value make_string(const wchar_t* str);

wchar_t* get_string_data(EO_object* obj);

EO_object* make_inner_application(EO_object* parent, EO_object* (*eval)(EO_object*));

EO_object* make_array(int length);

EO_object* array_at(EO_object* arr, int i);

int get_array_length(EO_object* obj);

EO_object* make_object_from_arg(char* arg);

#endif  // OBJECT_H
