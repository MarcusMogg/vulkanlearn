#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vkengine {
class Camera {
 public:
  Camera(/* args */) {}
  ~Camera() {}

  inline static const glm::vec3 X{1, 0, 0};
  inline static const glm::vec3 Y{0, 1, 0};
  inline static const glm::vec3 Z{0, 0, 1};
  static constexpr float        MIN_FOV{10.0f};
  static constexpr float        MAX_FOV{89.0f};
  static constexpr int          MAIN_VIEW_MATRIX_INDEX{0};

  void Move(glm::vec3 delta);
  void Rotate(glm::vec2 delta);
  void Zoom(float offset);
  void LookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

  void SetFarNear(float far, float near);
  void SetAspect(float aspect);

  glm::vec3 position() const { return position_; }
  glm::quat rotation() const { return rotation_; }
  glm::vec3 forward() const { return (inv_rotation_ * Y); }
  glm::vec3 up() const { return (inv_rotation_ * Z); }
  glm::vec3 right() const { return (inv_rotation_ * X); }

  glm::mat4 GetViewMatrix();
  glm::mat4 GetPersProjMatrix() const;
  glm::mat4 GetLookAtMatrix() const;

 private:
  glm::vec3 position_{0.0f, 0.0f, 0.0f};
  glm::quat rotation_;
  glm::quat inv_rotation_;
  glm::vec3 up_axis_{Z};
  float     znear_{1000.0f};
  float     zfar_{0.1f};
  float     aspect_{0.f};
};

}  // namespace vkengine
