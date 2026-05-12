#
# @author Tobias Weber
# @date may-2026
# @license see 'LICENSE.GPL' file
#

project(bt)
cmake_minimum_required(VERSION 3.5)
enable_language(CXX)


find_package(Boost REQUIRED)
find_package(Qt6Core REQUIRED)
find_package(Qt6Bluetooth REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-std=c++20 -Wall -Wextra)

add_definitions(${Boost_CXX_FLAGS})

include_directories(
	"${PROJECT_SOURCE_DIR}"
	"${Boost_INCLUDE_DIRS}/.."
)

add_executable(bt main.cpp)

target_link_libraries(bt
	"${Boost_LIBRARIES}"
	Qt6::Core Qt6::Bluetooth Qt6::QDarwinBluetoothPermissionPlugin
)
