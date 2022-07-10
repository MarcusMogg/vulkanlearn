#pragma once

#include <stdint.h>

#include <type_traits>

namespace vkengine {

// for warning C4101 unreferenced-local-variable
template <typename T>
void inline UnUsedVariable(const T&) {}

template <typename T>
uint32_t ToU32(const T t) {
  static_assert(std::is_arithmetic<T>::value, "T must be numeric");
  return static_cast<uint32_t>(t);
}

}  // namespace vkengine
