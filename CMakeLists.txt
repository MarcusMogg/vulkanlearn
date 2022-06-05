# CMakeList.txt: vulkanlearn 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("vulkanlearn")

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")
find_package(tinyobjloader CONFIG REQUIRED)

    
# 将源代码添加到此项目的可执行文件。
add_executable (vulkanlearn 
	"vulkanlearn.cpp" "vulkanlearn.h" 
	"app/application.h"  "app/application.cc" 
	"app/hellotriangleapplication.h" "app/hellotriangleapplication.cc"
	"util/log.h" 
	"util/assert_exception.h" "util/assert_exception.cc" 
	"app/validationlayer.h" "app/validationlayer.cc" 
	"shaders/shaderloader.h" "shaders/shaderloader.cc" 
	"app/pipeline.h" "app/pipeline.cc" 
	"app/buffer_object.h" 
	"texture/texture.h" "texture/texture.cc"
	"resources/model.h" "resources/model.cc" 
	"app/objmodelapplication.h" "app/objmodelapplication.cc")

target_compile_features(vulkanlearn PRIVATE cxx_std_17)

target_include_directories(vulkanlearn PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(vulkanlearn
	PRIVATE
		glfw
		Vulkan::Vulkan
		fmt::fmt
		tinyobjloader::tinyobjloader
)

# TODO: 如有需要，请添加测试并安装目标。
