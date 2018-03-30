#
# @author Tobias Weber
# @date nov-2017
# @license: see 'LICENSE.GPL' file
#

project(qtglwin)
cmake_minimum_required(VERSION 3.0)

find_package(Boost REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-std=c++17)
add_definitions(${Boost_CXX_FLAGS})
include_directories("${PROJECT_SOURCE_DIR}" "${Boost_INCLUDE_DIRS}/..")

add_executable(qttst qttst.cpp qttst.h)
target_link_libraries(qttst ${Boost_LIBRARIES})
qt5_use_modules(qttst Core Gui Widgets)
