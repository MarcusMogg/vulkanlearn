#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace vkengine {
struct PointLight {
  glm::vec3 position;
  glm::vec3 flux;  // radiant flux in W

  // calculate an appropriate radius for light culling
  // a windowing function in the shader will perform a smooth transition to zero
  // this is not physically based and usually artist controlled
  float CalculateRadius() const {
    // radius = where attenuation would lead to an intensity of 1W/m^2
    const float INTENSITY_CUTOFF    = 1.0f;
    const float ATTENTUATION_CUTOFF = 0.05f;
    glm::vec3   intensity           = flux / (4.0f * glm::pi<float>());
    float       maxIntensity        = glm::max(intensity.x, intensity.y, intensity.z);
    float       attenuation =
        glm::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;
    return 1.0f / sqrtf(attenuation);
  }
};

struct AmbientLight {
  glm::vec3 irradiance;
};

struct PDirectionalLight {
  glm::vec3 direction;
  glm::vec3 color;
};

struct LightList {
  // vertex buffers seem to be aligned to 16 bytes
  struct PointLightVertex {
    glm::vec3 position;
    // radiant intensity in W/sr
    // can be calculated from radiant flux
    glm::vec3 intensity;
    float     radius;
  };
};

class PointLightList : public LightList {
 public:
  void init() {}
  void shutdown() {}

  // upload changes to GPU
  void update() {}

  std::vector<PointLight>    lights;
  std::vector<unsigned char> buffer;
};
}  // namespace vkengine
