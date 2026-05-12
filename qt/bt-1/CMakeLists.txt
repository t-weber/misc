#
# @author Tobias Weber
# @date may-2026
# @license see 'LICENSE.GPL' file
#

project(bt)
cmake_minimum_required(VERSION 3.5)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-std=c++20)
add_compile_options(-Wall -Wextra -Weffc++)

find_package(Boost REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Bluetooth)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)

add_definitions(${Boost_CXX_FLAGS})

include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${Boost_INCLUDE_DIRS}/..")

add_executable(bt bt.cpp bt.h)

target_link_libraries(bt
  ${Boost_LIBRARIES}
  Qt6::Core Qt6::Gui Qt6::Widgets
  Qt6::Bluetooth Qt6::QDarwinBluetoothPermissionPlugin
)
