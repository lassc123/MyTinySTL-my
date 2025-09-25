#ifndef MYTINYSTL_EXCEPTDEF_H
#define MYTINYSTL_EXCEPTDEF_H

#include <cassert>
#include <stdexcept>

namespace mystl {
#define MYSTL_DEBUG(expr) assert(expr)

#define THROW_LENGTH_ERROR_IF(expr, what)                                      \
  if ((expr))                                                                  \
  throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what)                                      \
  if ((expr))                                                                  \
  throw std::out_of_range(what)

#define THROW_RUNTIME_ERROR(expr, what)                                        \
  if ((expr))                                                                  \
  throw std::runtime_error(what)
} // namespace mystl

#endif //! MYTINYSTL_EXCEPTDEF_H