#pragma once

namespace vkengine {

template <typename T>
class Singleton {
 protected:
  Singleton() {}
  ~Singleton() {}

 public:
  static T& GetInstance() {
    static T instance;
    return instance;
  }

  Singleton(const Singleton&) = delete;
  Singleton(Singleton&&)      = delete;
  Singleton& operator=(const Singleton&) = delete;
  Singleton& operator=(Singleton&&) = delete;
};

}  // namespace vkengine