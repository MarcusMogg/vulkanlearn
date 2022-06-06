

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")
find_package(tinyobjloader CONFIG REQUIRED)

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE CC_FILES "*.cc")

# 将源代码添加到此项目的可执行文件。
add_library(vkengine ${HEADER_FILES} ${CC_FILES} )

target_compile_features(vkengine PRIVATE cxx_std_17)

target_include_directories(vkengine PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(vkengine PRIVATE source)
target_link_libraries(vkengine
	PRIVATE
		glfw
		Vulkan::Vulkan
		fmt::fmt
		tinyobjloader::tinyobjloader
)
