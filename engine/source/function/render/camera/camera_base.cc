#pragma once

#include "function/render/camera/camera_base.h"

namespace vkengine {

void Camera::Move(glm::vec3 delta) {}
void Camera::Rotate(glm::vec2 delta) {}
void Camera::Zoom(float offset) {}
void Camera::LookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {}

void Camera::SetFarNear(float far, float near) {}
void Camera::SetAspect(float aspect) {}

glm::mat4 Camera::GetViewMatrix() {}
glm::mat4 Camera::GetPersProjMatrix() const {}
glm::mat4 Camera::GetLookAtMatrix() const {}

}  // namespace vkengine
