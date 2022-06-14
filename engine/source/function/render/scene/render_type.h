#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"

namespace vkengine {
#pragma region VulkanBufferType

using VertexType = glm::vec3;
using UvType     = glm::vec2;

struct VulkanVertexData {
  VertexType position;
  VertexType normal;
  VertexType tangent;
  UvType     texcoord;

  bool operator==(const VulkanVertexData& rhs) const { return position == rhs.position; }

  struct HasHValue {
    size_t operator()(const VulkanVertexData& rhs) const {
      size_t seed = 0;
      for (int i = 0; i < rhs.position.length(); ++i) {
        auto elem = rhs.position[i];
        seed ^= std::hash<float>()(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };

  static VkVertexInputBindingDescription GetBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding   = 0;
    bindingDescription.stride    = sizeof(VulkanVertexData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;

    attributeDescriptions[0].binding  = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(VulkanVertexData, position);

    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(VulkanVertexData, normal);

    attributeDescriptions[2].binding  = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset   = offsetof(VulkanVertexData, tangent);

    attributeDescriptions[2].binding  = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset   = offsetof(VulkanVertexData, texcoord);

    return attributeDescriptions;
  }
};

struct VulkanVertexBuffer {
  uint32_t mesh_vertex_count;
  uint32_t mesh_index_count;

  VkBuffer       mesh_vertex_buffer = VK_NULL_HANDLE;
  VkDeviceMemory mesh_vertex_memory = VK_NULL_HANDLE;
  VkBuffer       mesh_index_buffer  = VK_NULL_HANDLE;
  VkDeviceMemory mesh_index_memory  = VK_NULL_HANDLE;

  VkDescriptorSet mesh_vertex_descriptor_set;
};

struct VulkanMaterialBuffer {
  VkImage        base_color_image        = VK_NULL_HANDLE;
  VkImageView    base_color_image_view   = VK_NULL_HANDLE;
  VkDeviceMemory base_color_image_memory = VK_NULL_HANDLE;

  VkDescriptorSet material_descriptor_set;
};

#pragma endregion

#pragma region VulkanShaderUniformBufferObject

// ubo UniformBufferObject
struct VkPerMaterialUbo {
  glm::vec4 base_color_factor = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct VkPerframeStorageUbo {
  glm::mat4 proj_view_matrix;
  glm::vec3 camera_position;
  float     _padding_1;
};

struct VkAllStorageUbo {
  VkPerframeStorageUbo per_frame_ubo;
};
#pragma endregion

#pragma region RendererLogicType

struct RenderEntity {
  uint32_t instance_id{0};

  // mesh
  size_t    mesh_asset_id{0};
  glm::mat4 model_matrix;

  // material
  size_t    material_asset_id{0};
  glm::vec4 base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
};

struct RenderMeshSource {
  std::string mesh_file;

  bool operator==(const RenderMeshSource& rhs) const { return mesh_file == rhs.mesh_file; }

  struct HasHValue {
    size_t operator()(const RenderMeshSource& rhs) const {
      return std::hash<std::string>{}(rhs.mesh_file);
    }
  };
};

struct RenderMeshData {
  std::vector<VulkanVertexData> vertex_buffer;
  std::vector<uint32_t>         index_buffer;
};
struct RenderMesh {
  RenderMeshData static_mesh_data;
};

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

struct RenderMaterialSource {
  std::string base_color_file;

  bool operator==(const RenderMaterialSource& rhs) const {
    return base_color_file == rhs.base_color_file;
  }
  struct HasHValue {
    size_t operator()(const RenderMaterialSource& rhs) const {
      size_t h0 = std::hash<std::string>{}(rhs.base_color_file);
      return h0;
      // return (((h0 ^ (h1 << 1)) ^ (h2 << 1)) ^ (h3 << 1)) ^ (h4 << 1);
    }
  };
};

struct RenderMaterialData {
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

struct RenderMaterial {
  std::shared_ptr<RenderMaterialData> base_color_texture;
};

struct StorageBuffer {
  VkBuffer       global_ubo_buffer;
  VkDeviceMemory global_ubo_memory;

  std::vector<VkAllStorageUbo> ubo;

  VkBuffer       global_null_descriptor_storage_buffer;
  VkDeviceMemory global_null_descriptor_storage_buffer_memory;
};

#pragma endregion

}  // namespace vkengine
