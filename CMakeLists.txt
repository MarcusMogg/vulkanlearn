# CMakeList.txt: vulkanlearn 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("vulkanlearn")

add_subdirectory(engine)
    
# 将源代码添加到此项目的可执行文件。
add_executable (vulkanlearn 
	"vulkanlearn.cpp" "vulkanlearn.h" )

target_compile_features(vulkanlearn PRIVATE cxx_std_17)

target_include_directories(vulkanlearn PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(vulkanlearn
	PRIVATE
		vkengine
)

# TODO: 如有需要，请添加测试并安装目标。
