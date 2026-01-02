#
# @author Tobias Weber
# @date 26-dec-2025
# @license: see 'LICENSE.GPL' file
#

project(qttst)
cmake_minimum_required(VERSION 3.5)

find_package(Boost REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-std=c++20)
add_definitions(${Boost_CXX_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}"
  "${Boost_INCLUDE_DIRS}"
  "${Qt6Core_INCLUDE_DIRS}" "${Qt6Gui_INCLUDE_DIRS}" "${Qt6Widgets_INCLUDE_DIRS}")

add_executable(qttst qttst.cpp)
target_link_libraries(qttst ${Boost_LIBRARIES} Qt6::Core Qt6::Gui Qt6::Widgets)
