#
# @author Tobias Weber
# @date nov-2017
# @license: see 'LICENSE' file
#

project(qtglwin)
cmake_minimum_required(VERSION 3.0)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-std=c++17)
include_directories("${PROJECT_SOURCE_DIR}")

add_executable(qttst qttst.cpp qttst.h)
target_link_libraries(qttst Qt5::Core Qt5::Widgets)
