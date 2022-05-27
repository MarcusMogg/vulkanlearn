#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../util/assert_exception.h"
namespace vklearn {

Texture::~Texture() { stbi_image_free(buffer_); }

void Texture::LoadTex(const std::string& name) {
  buffer_ = stbi_load(name.c_str(), &width_, &height_, &channels_, STBI_rgb_alpha);
  ASSERT_EXECPTION(buffer_ == nullptr).SetErrorMessage("failed to load texture!").Throw();
}

}  // namespace vklearn