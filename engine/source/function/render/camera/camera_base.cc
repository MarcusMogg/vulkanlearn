#pragma once

#include "function/render/camera/camera_base.h"

#include "core/utils/ccn_utils.h"

namespace vkengine {

void Camera::Move(glm::vec3 delta) { UnUsedVariable(delta); }
void Camera::Rotate(glm::vec2 delta) { UnUsedVariable(delta); }
void Camera::Zoom(float offset) { UnUsedVariable(offset); }
void Camera::LookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
  UnUsedVariable(position);
  UnUsedVariable(target);
  UnUsedVariable(up);
}

void Camera::SetFarNear(float far, float near) {
  UnUsedVariable(far);
  UnUsedVariable(near);
}
void Camera::SetAspect(float aspect) { UnUsedVariable(aspect); }

glm::mat4 Camera::GetViewMatrix() {
  return glm::lookAt(
      glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}
glm::mat4 Camera::GetPersProjMatrix() const {
  return glm::perspective(glm::radians(45.0f), 1.5f, 0.1f, 10.0f);
}
glm::mat4 Camera::GetLookAtMatrix() const { return glm::mat4(); }

}  // namespace vkengine
