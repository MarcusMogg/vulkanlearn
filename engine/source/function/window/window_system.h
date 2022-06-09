#pragma once

class GLFWwindow;

namespace vkengine {
struct WindowCreateInfo {
  int         width{1280};
  int         height{720};
  const char* title{"Vk"};
  bool        fullscreen{false};
};

class WindowSystem {
 public:
  WindowSystem(const WindowCreateInfo& info);
  ~WindowSystem();

  GLFWwindow* GetWindow() const;
};

}  // namespace vkengine
