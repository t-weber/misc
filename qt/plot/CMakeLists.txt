#
# @author Tobias Weber
# @date 30-mar-2018
# @license: see 'LICENSE.GPL' file
#

project(plottst)
cmake_minimum_required(VERSION 3.0)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-std=c++17)
include_directories("${PROJECT_SOURCE_DIR}")

add_executable(qcp qcp.cpp qcp.h
	qcustomplot/qcustomplot.cpp qcustomplot/qcustomplot.h)
target_link_libraries(qcp Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)
