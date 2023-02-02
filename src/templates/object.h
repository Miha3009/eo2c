#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>

typedef int Tag;

struct EO_object;

struct EO_head {
  EO_object* (*eval)(EO_object*);
  int id;
  int parent_offset;
  int attr_count;
  int varargs;
  size_t (*size)(EO_object*);
  const std::unordered_map<Tag, int>* offset_table;
};

struct EO_object {
  EO_head  head;
};

EO_object* get_parent(EO_object* obj);

EO_object* get_sub(EO_object* obj, Tag tag);

EO_object* evaluate(EO_object* obj);

void init_head(EO_object* obj, int parent_offset,
               EO_object* (*eval)(EO_object*),
               size_t (*size)(EO_object*),
               const std::unordered_map<Tag, int>* offset_table,
               int varargs);

typedef char* StackPos;

struct Stack {
  char data[16384*16384];
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
  std::cout << "RESTORE TO: " << pos - stack.data << std::endl;
  stack.pos = pos;
}

inline EO_object* stack_move(EO_object* obj) {
  size_t size = obj->head.size(obj);
  std::cout << (char*)obj-stack.data << " -> " << stack.pos-stack.data << "\n";
  std::memcpy(stack.pos, obj, size);
  return stack_alloc(size);
}

template <typename... Attributes>
void add_attributes(EO_object* obj, Attributes... attrs) {
  EO_object* attrs_[sizeof...(attrs)] = {attrs...};
  EO_object** obj_attr = (EO_object**)(obj + 1) + obj->head.attr_count;
  int i = 0;
  for(; obj->head.varargs != obj->head.attr_count; ++i) {
    if(i == sizeof...(attrs)) {
      return;
    }
    *(obj_attr + i) = attrs_[i];
    ++obj->head.attr_count;
  }
  EO_object*** a = (EO_object***)(obj + 1) + obj->head.varargs;
  EO_object** b = (EO_object**)stack_alloc(sizeof(EO_object*) * (sizeof...(attrs) - i));
  *(int*)(a + 1) = sizeof...(attrs) - i;
  *a = b;
  int j = 0;
  for(; i < sizeof...(attrs); ++i, ++j) {
    *(b + j) = attrs_[i];
  }
}

void reset_attributes(EO_object* obj);

EO_object* clone(EO_object* obj);

struct bytes_value {
  int length;
  unsigned char* data;
};

template <typename... Bytes>
bytes_value make_bytes(Bytes... bytes) {
  unsigned char bytes_[sizeof...(bytes)] = {bytes...};
  bytes_value result;
  result.length = sizeof...(Bytes);
  result.data = (unsigned char*)stack_alloc(sizeof...(bytes));
  for(int i = 0; i < sizeof...(bytes); ++i) {
    result.data[i] = bytes_[i];
  }
  return result;
}

#endif  // OBJECT_H
