#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "core/exception/assert_exception.h"

namespace vkengine {
class ObjectPool {
 public:
  using VoidPointer = std::shared_ptr<void>;

  ObjectPool()          = default;
  virtual ~ObjectPool() = default;

  virtual VoidPointer&                Create(const std::string_view& name)     = 0;
  virtual std::optional<VoidPointer>  Find(const std::string_view& name) const = 0;
  virtual std::optional<VoidPointer>  Remove(const std::string_view& name)     = 0;
  virtual std::shared_ptr<ObjectPool> Clone() const                            = 0;
};

class DefaultObjectPool : public ObjectPool {
 private:
  std::map<std::string, VoidPointer, std::less<>> pool_;

 public:
  VoidPointer&                Create(const std::string_view& name) override;
  std::optional<VoidPointer>  Find(const std::string_view& name) const override;
  std::optional<VoidPointer>  Remove(const std::string_view& name) override;
  std::shared_ptr<ObjectPool> Clone() const override;
};

class IObjectPool {
 public:
  template <typename ObjectType, typename... Args>
  std::shared_ptr<ObjectType> CreateObject(const std::string_view& name, Args&&... args) {
    auto& r = pool_->Create(name);
    auto  p = std::make_shared<ObjectType>(std::forward<Args>(args)...);
    r       = std::static_pointer_cast<void>(p);
    return p;
  }

  template <typename ObjectType>
  std::shared_ptr<ObjectType> MustFindObject(const std::string_view& name) const {
    auto res = pool_->Find(name);
    ASSERT_EXECPTION(!res.has_value()).SetErrorMessage(fmt::format("can't find{}", name)).Throw();
    return std::static_pointer_cast<ObjectType>(res.value());
  }

  template <typename ObjectType>
  std::optional<std::shared_ptr<ObjectType>> RemoveObject(const std::string_view& name) {
    auto res = pool_->Remove(name);
    if (res.has_value()) {
      return {std::static_pointer_cast<ObjectType>(res.value())};
    }
    return {};
  }

  void SetObjectPool(std::shared_ptr<ObjectPool> pointer) { pool_ = pointer; }

 private:
  std::shared_ptr<ObjectPool> pool_;
};

}  // namespace vkengine
