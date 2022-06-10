#pragma once

#include <cstdint>
#include <vector>

namespace vkengine {
enum class PixelFormat : uint8_t {
  UNKNOWN = 0,
  R8G8B8_UNORM,
  R8G8B8_SRGB,
  R8G8B8A8_UNORM,
  R8G8B8A8_SRGB,
  R32G32_FLOAT,
  R32G32B32_FLOAT,
  R32G32B32A32_FLOAT
};

enum class ImageType : uint8_t { UNKNOWM = 0, IMAGE_2D };

struct TextureData {
  uint32_t             width        = 0;
  uint32_t             height       = 0;
  uint32_t             depth        = 0;
  uint32_t             mip_levels   = 0;
  uint32_t             array_layers = 0;
  PixelFormat          format       = PixelFormat::UNKNOWN;
  ImageType            type         = ImageType::UNKNOWM;
  std::vector<uint8_t> pixels;

  bool IsValid() const { return pixels.empty(); }
};

struct MeshVertexDataDefinition {
  float x, y, z;     // position
  float nx, ny, nz;  // normal
  float tx, ty, tz;  // tangent
  float u, v;        // UV coordinates
};

struct MeshVertexBindingDataDefinition {
  int   index0  = 0;
  int   index1  = 0;
  int   index2  = 0;
  int   index3  = 0;
  float weight0 = 0.f;
  float weight1 = 0.f;
  float weight2 = 0.f;
  float weight3 = 0.f;
};
}  // namespace vkengine
