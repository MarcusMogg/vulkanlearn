#pragma once

namespace vkengine {

class Engine {
 private:
  /* data */
 public:
  Engine(/* args */) = default;
  ~Engine();

  void Start();
  void Shutdown();
};

}  // namespace vkengine
