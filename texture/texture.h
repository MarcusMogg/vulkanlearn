#pragma once
#include <algorithm>
#include <cmath>
#include <string>

namespace vklearn {
class Texture {
 public:
  ~Texture();
  void LoadTex(const std::string& name);
  const void* GetBuffer() const { return buffer_; }
  const std::tuple<int, int, int> GetSize() const { return {width_, height_, channels_}; }
  const uint32_t GetImageBytes() const { return width_ * height_ * channels_; }

  const uint32_t GetMipLevel() const {
    return static_cast<uint32_t>(std::floor(std::log2(std::max(width_, height_)))) + 1;
  }

 private:
  int width_, height_, channels_;
  void* buffer_;
};
}  // namespace vklearn