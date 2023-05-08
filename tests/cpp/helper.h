#include "object.h"
#include "translation_unit.h"

TranslationUnit makeUnit();

class NullOstream : public std::ostream {
public:
  NullOstream() : std::ostream() {}
};

template <class T>
const NullOstream &operator<<(NullOstream &&os, const T &value) { 
  return os;
}
