#include "object.h"

Stack stack;
int obj_count = 0;

EO_object* get_parent(EO_object* obj) {
  return (EO_object*)((StackPos)obj - obj->head.parent_offset);
}

EO_object* get_sub(EO_object* obj, Tag tag) {
  EO_object* result = (EO_object*)((StackPos)obj + obj->head.offset_table->at(tag));
  if(result < (EO_object*)((EO_object**)(obj + 1) + obj->head.attr_count)) {
    return *(EO_object**)result;
  }
  return (EO_object*)result;
}

EO_object* evaluate(EO_object* obj) {
  return obj->head.eval(obj);
}

void init_head(EO_object* obj, int parent_offset,
               EO_object* (*eval)(EO_object*),
               size_t (*size)(EO_object*),
               const std::unordered_map<Tag, int>* offset_table,
               int varargs) {
  obj->head.id = obj_count++;
  obj->head.attr_count = 0;
  obj->head.varargs = varargs;
  obj->head.parent_offset = parent_offset;
  obj->head.eval = eval;
  obj->head.size = size;
  obj->head.offset_table = offset_table;
}

void reset_attributes(EO_object* obj) {
  obj->head.attr_count = 0;
  if(obj->head.varargs != -1) {
      *(int*)((EO_object***)(obj + 1) + obj->head.varargs + 1) = 0;
  }
}

EO_object* clone(EO_object* obj) {
  EO_object* obj_clone = stack_alloc(obj->head.size(obj));
  std::memcpy(obj_clone, obj, obj->head.size(obj));
  return obj_clone;
}

EO_object* make_meta_object(EO_object* (*eval)(EO_object*)) {
    EO_object* meta_obj = stack_alloc(sizeof(EO_meta_object));
    meta_obj->head.eval = eval;
    return meta_obj;

}