
#include "function/render/resource/render_resource_base.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core/exception/assert_exception.h"
#include "macro.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace vkengine {

RenderResourceBase::BoudingBox& RenderResourceBase::GetCachedBoudingBox(
    const MeshSourceDesc& source) {
  bounding_box_cache_map.try_emplace(source, BoudingBox{});
  return bounding_box_cache_map[source];
}

StaticMeshData RenderResourceBase::LoadStaticMesh(
    const std::string& mesh_file, BoudingBox& bounding_box) {
  StaticMeshData mesh_data;

  tinyobj::ObjReader       reader;
  tinyobj::ObjReaderConfig reader_config;
  reader_config.vertex_color = false;
  ASSERT_EXECPTION(!reader.ParseFromFile(mesh_file, reader_config))
      .SetErrorMessage(fmt::format("load mesh {} fail, error : {} ", mesh_file, reader.Error()))
      .Throw();

  if (!reader.Warning().empty()) {
    LogWarn("loadMesh {} warning, warning: {}", mesh_file, reader.Warning());
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();

  std::unordered_map<MeshVertexDataDefinition, uint32_t, MeshVertexDataDefinition::HasHValue>
      mesh_vertices;
  for (const auto& shape : shapes) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shape.mesh.num_face_vertices[f]);

      bool with_normal   = true;
      bool with_texcoord = true;

      VertexType vertex[3];
      VertexType normal[3];
      UvType     uv[3];

      // only deals with triangle faces
      if (fv != 3) {
        continue;
      }

      // expanding vertex data is not efficient and is for testing purposes only
      for (size_t v = 0; v < fv; v++) {
        auto idx = shape.mesh.indices[index_offset + v];
        auto vx  = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        auto vy  = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        auto vz  = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

        vertex[v] = {static_cast<float>(vx), static_cast<float>(vy), static_cast<float>(vz)};

        // TODO: bounding_box
        bounding_box = 1;
        // bounding_box.extend(vertex[v]);

        if (idx.normal_index >= 0) {
          auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

          normal[v] = {static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz)};
        } else {
          with_normal = false;
        }

        if (idx.texcoord_index >= 0) {
          auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

          uv[v] = {static_cast<float>(tx), static_cast<float>(ty)};
        } else {
          with_texcoord = false;
        }
      }
      index_offset += fv;

      if (!with_normal) {
        VertexType v0 = vertex[1] - vertex[0];
        VertexType v1 = vertex[2] - vertex[1];
        normal[0]     = glm::normalize(glm::cross(v0, v1));
        normal[1]     = normal[0];
        normal[2]     = normal[0];
      }

      if (!with_texcoord) {
        uv[0] = UvType(0.5f, 0.5f);
        uv[1] = UvType(0.5f, 0.5f);
        uv[2] = UvType(0.5f, 0.5f);
      }

      VertexType tangent{1, 0, 0};
      {
        VertexType edge1    = vertex[1] - vertex[0];
        VertexType edge2    = vertex[2] - vertex[1];
        UvType     deltaUV1 = uv[1] - uv[0];
        UvType     deltaUV2 = uv[2] - uv[1];

        // auto divide = deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1];
        // if (divide >= 0.0f && divide < 0.000001f) {
        //   divide = 0.000001f;
        // } else if (divide < 0.0f && divide > -0.000001f) {
        //   divide = -0.000001f;
        // }

        // float df = 1.0f / divide;

        // tbn
        // [e1,e2] = [t,b] * [u v]
        // https://www.zhihu.com/column/c_1249465121615204352
        // Matrix<float, 3, 2> e;
        // e.col(0) = edge1;
        // e.col(1) = edge2;
        // Vector2f uv_inv_col1{deltaUV2[1], -deltaUV1[1]};
        // tangent = (e * uv_inv_col1).col(0).normalized();

        tangent = {
            (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]),
            (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]),
            (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2])};
        tangent = glm::normalize(tangent);
      }

      for (size_t i = 0; i < 3; i++) {
        MeshVertexDataDefinition mesh_vert{};

        mesh_vert.position = vertex[i];
        mesh_vert.normal   = normal[i];
        mesh_vert.uv       = uv[i];
        mesh_vert.tangent  = tangent;

        const auto it = mesh_vertices.find(mesh_vert);
        if (it == mesh_vertices.end()) {
          mesh_vertices[mesh_vert] = static_cast<uint32_t>(mesh_data.vertex_buffer.size());
          mesh_data.index_buffer.emplace_back(
              static_cast<uint32_t>(mesh_data.vertex_buffer.size()));
          mesh_data.vertex_buffer.emplace_back(mesh_vert);

        } else {
          mesh_data.index_buffer.emplace_back(it->second);
        }
      }
    }
  }

  return mesh_data;
}

RenderMeshData RenderResourceBase::LoadMeshData(
    const MeshSourceDesc& source, BoudingBox& bounding_box) {
  RenderMeshData ret;

  if (std::filesystem::path(source.mesh_file).extension() == ".obj") {
    ret.static_mesh_data = LoadStaticMesh(source.mesh_file, bounding_box);
  }
  GetCachedBoudingBox(source) = bounding_box;
  return ret;
}
RenderMaterialData RenderResourceBase::LoadMaterialData(const MaterialSourceDesc& source) {
  RenderMaterialData ret;
  ret.base_color_texture         = LoadTexture(source.base_color_file, true);
  ret.metallic_roughness_texture = LoadTexture(source.metallic_roughness_file);
  ret.normal_texture             = LoadTexture(source.normal_file);
  ret.occlusion_texture          = LoadTexture(source.occlusion_file);
  ret.emissive_texture           = LoadTexture(source.emissive_file);
  return ret;
}

std::shared_ptr<TextureData> RenderResourceBase::LoadTextureHDR(
    const std::string& file, int desired_channels) {
  int    iw, ih, n;
  float* buffer = stbi_loadf(file.c_str(), &iw, &ih, &n, desired_channels);

  if (!buffer) {
    return nullptr;
  }

  std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();

  texture->pixels       = buffer;
  texture->width        = iw;
  texture->height       = ih;
  texture->depth        = 1;
  texture->array_layers = 1;
  texture->mip_levels   = 1;
  texture->type         = ImageType::IMAGE_2D;
  switch (desired_channels) {
    case 2:
      texture->format = PixelFormat::R32G32_FLOAT;
      break;
    case 4:
      texture->format = PixelFormat::R32G32B32A32_FLOAT;
      break;
    default:
      // three component format is not supported in some vulkan driver implementations
      throw std::runtime_error("unsupported channels number");
      break;
  }
  return texture;
}
std::shared_ptr<TextureData> RenderResourceBase::LoadTexture(
    const std::string& file, bool is_srgb) {
  int            iw, ih, n;
  unsigned char* buffer = stbi_load(file.c_str(), &iw, &ih, &n, 4);

  if (!buffer) {
    return nullptr;
  }

  std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();

  texture->pixels       = buffer;
  texture->width        = iw;
  texture->height       = ih;
  texture->format       = (is_srgb) ? PixelFormat::R8G8B8A8_SRGB : PixelFormat::R8G8B8A8_UNORM;
  texture->depth        = 1;
  texture->array_layers = 1;
  texture->mip_levels   = 1;
  texture->type         = ImageType::IMAGE_2D;

  return texture;
}

}  // namespace vkengine
