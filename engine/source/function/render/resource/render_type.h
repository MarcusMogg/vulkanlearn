#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace vkengine {

using VertexType = glm::vec3;
using UvType     = glm::vec2;

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
  uint32_t    width        = 0;
  uint32_t    height       = 0;
  uint32_t    depth        = 0;
  uint32_t    mip_levels   = 0;
  uint32_t    array_layers = 0;
  PixelFormat format       = PixelFormat::UNKNOWN;
  ImageType   type         = ImageType::UNKNOWM;
  void*       pixels       = nullptr;

  bool IsValid() const { return pixels != nullptr; }
};

struct MeshVertexDataDefinition {
  VertexType position;
  VertexType normal;
  VertexType tangent;
  UvType     uv;

  bool operator==(const MeshVertexDataDefinition& rhs) const { return position == rhs.position; }

  struct HasHValue {
    size_t operator()(const MeshVertexDataDefinition& rhs) const {
      size_t seed = 0;
      for (int i = 0; i < rhs.position.length(); ++i) {
        auto elem = rhs.position[i];
        seed ^= std::hash<float>()(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };
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

struct MeshSourceDesc {
  std::string mesh_file;

  bool operator==(const MeshSourceDesc& rhs) const { return mesh_file == rhs.mesh_file; }

  struct HasHValue {
    size_t operator()(const MeshSourceDesc& rhs) const {
      return std::hash<std::string>{}(rhs.mesh_file);
    }
  };
};

struct MaterialSourceDesc {
  std::string base_color_file;
  std::string metallic_roughness_file;
  std::string normal_file;
  std::string occlusion_file;
  std::string emissive_file;

  bool operator==(const MaterialSourceDesc& rhs) const {
    return base_color_file == rhs.base_color_file &&
           metallic_roughness_file == rhs.metallic_roughness_file &&
           normal_file == rhs.normal_file && occlusion_file == rhs.occlusion_file &&
           emissive_file == rhs.emissive_file;
  }
  struct HasHValue {
    size_t operator()(const MaterialSourceDesc& rhs) const {
      size_t h0 = std::hash<std::string>{}(rhs.base_color_file);
      size_t h1 = std::hash<std::string>{}(rhs.metallic_roughness_file);
      size_t h2 = std::hash<std::string>{}(rhs.normal_file);
      size_t h3 = std::hash<std::string>{}(rhs.occlusion_file);
      size_t h4 = std::hash<std::string>{}(rhs.emissive_file);
      return (((h0 ^ (h1 << 1)) ^ (h2 << 1)) ^ (h3 << 1)) ^ (h4 << 1);
    }
  };
};

struct StaticMeshData {
  std::vector<MeshVertexDataDefinition> vertex_buffer;
  std::vector<uint32_t>                 index_buffer;
};

struct RenderMeshData {
  StaticMeshData       static_mesh_data;
  std::vector<uint8_t> skeleton_binding_buffer;
};

struct RenderMaterialData {
  std::shared_ptr<TextureData> base_color_texture;
  std::shared_ptr<TextureData> metallic_roughness_texture;
  std::shared_ptr<TextureData> normal_texture;
  std::shared_ptr<TextureData> occlusion_texture;
  std::shared_ptr<TextureData> emissive_texture;
};

}  // namespace vkengine
