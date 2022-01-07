#
# @author Tobias Weber
# @date Nov-2017, Jan-2022
# @license see 'LICENSE.GPL' file
#

project(gltst)
cmake_minimum_required(VERSION 3.0)

find_package(Boost REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-std=c++20 -Wall -Wextra)
add_definitions(${Boost_CXX_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}" "${Boost_INCLUDE_DIRS}/..")

add_executable(glplot
	glplot.cpp glplot.h
	test.cpp)
target_link_libraries(glplot ${Boost_LIBRARIES}
	Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
