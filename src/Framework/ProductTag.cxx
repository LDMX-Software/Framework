#include "Framework/ProductTag.h"

std::ostream& operator<<(std::ostream& s, const framework::ProductTag& pt) {
  return s << pt.name() << "_" << pt.passname() << "_" << pt.type();
}
