#pragma once
#include <string>

namespace vklearn {
class Texture {
 public:
  ~Texture();
  void LoadTex(const std::string& name);
  const void* GetBuffer() const { return buffer_; }
  const std::tuple<int, int, int> GetSize() const { return {width_, height_, channels_}; }
  const uint32_t GetImageBytes() const { return width_ * height_ * channels_; }

 private:
  int width_, height_, channels_;
  void* buffer_;
};
}  // namespace vklearn