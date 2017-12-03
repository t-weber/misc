#
# @author Tobias Weber
# @date nov-2017
# @license: see 'LICENSE' file
#

project(qtglwin)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

find_package(Boost REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-std=c++17)
add_definitions(${Boost_CXX_FLAGS})

include_directories("."
	"${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
	"${CMAKE_CURRENT_BINARY_DIR}/qttst_autogen"
	"${Boost_INCLUDE_DIRS}/..")

message("Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(qttst qttst.cpp qttst.h)
target_link_libraries(qttst ${Boost_LIBRARIES})
qt5_use_modules(qttst Core Gui Widgets OpenGL)
