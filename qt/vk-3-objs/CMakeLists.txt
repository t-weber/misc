#
# @author Tobias Weber
# @date feb-2021
# @license: see 'LICENSE.GPL' file
#

project(qtvkwin)
cmake_minimum_required(VERSION 3.0)
enable_language(CXX)


find_package(Boost REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-std=c++20 -Wall -Wextra)

add_definitions(${Boost_CXX_FLAGS})
include_directories("${PROJECT_SOURCE_DIR}" "${Boost_INCLUDE_DIRS}/..")


add_executable(qttst qttst.cpp qttst.h)

target_link_libraries(qttst
	${Boost_LIBRARIES}
	Qt5::Core Qt5::Widgets Qt5::Gui)


execute_process(
	COMMAND glslangValidator -V100 -o vert.spv ${PROJECT_SOURCE_DIR}/shader.vert
	COMMAND glslangValidator -V100 -o frag.spv ${PROJECT_SOURCE_DIR}/shader.frag

	# alternatively:
	#COMMAND glslc --target-env=vulkan1.0 -o vert.spv ${PROJECT_SOURCE_DIR}/shader.vert
	#COMMAND glslc --target-env=vulkan1.0 -o frag.spv ${PROJECT_SOURCE_DIR}/shader.frag
)
