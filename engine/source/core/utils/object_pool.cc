#include "core/utils/object_pool.h"

namespace vkengine {
DefaultObjectPool::VoidPointer& DefaultObjectPool::Create(const std::string_view& name) {
  return pool_[std::string(name)];
}

std::optional<DefaultObjectPool::VoidPointer> DefaultObjectPool::Find(
    const std::string_view& name) const {
  const auto it = pool_.find(name);
  if (it == pool_.end()) {
    return {};
  }
  return {it->second};
}

std::optional<DefaultObjectPool::VoidPointer> DefaultObjectPool::Remove(
    const std::string_view& name) {
  const auto it = pool_.find(name);
  if (it == pool_.end()) {
    return {};
  }
  std::optional<DefaultObjectPool::VoidPointer> res{it->second};
  pool_.erase(it);
  return res;
}

std::shared_ptr<ObjectPool> DefaultObjectPool::Clone() const {
  return std::make_shared<DefaultObjectPool>(*this);
}
}  // namespace vkengine