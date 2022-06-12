#pragma once

#include "forward.h"
#include "function/render/resource/light.h"

namespace vkengine {
class RenderScene {
 public:
  // light
  AmbientLight      ambient_light;
  PDirectionalLight directional_light;
  PointLightList    point_light_list;

  // render entities
  std::vector<RenderEntity> render_entities;

 public:
  RenderScene() {}
  ~RenderScene() {}
};

}  // namespace vkengine
