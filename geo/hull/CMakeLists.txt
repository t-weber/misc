#
# @author Tobias Weber
# @date 15-aug-2020
# @license: see 'LICENSE.GPL' file
#

project(hull)
cmake_minimum_required(VERSION 3.0)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-std=c++20)

add_definitions(-Wall -Wextra)

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(hull hull.cpp hull.h geo.h)
target_link_libraries(hull Qt5::Core Qt5::Widgets -lqhull_r -lqhullcpp)
