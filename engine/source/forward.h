#pragma once

#include "core/exception/assert_exception.h"
#include "core/utils/ccn_utils.h"
#include "core/utils/object_pool.h"
#include "core/utils/singleton.h"

// forward declarations

namespace vkengine {

namespace vulkan {
class Device;
class Instance;
class PhysicalDevice;
class Queue;
class ValidationLayer;
}  // namespace vulkan

class WindowSystem;
class RenderSystem;
class LogSystem;
}  // namespace vkengine

class GLFWwindow;