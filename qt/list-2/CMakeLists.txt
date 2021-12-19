#
# @author Tobias Weber
# @date dec-2021
# @license: see 'LICENSE.GPL' file
#

project(qttst)
cmake_minimum_required(VERSION 3.0)

find_package(Boost REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTOMOC TRUE)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-std=c++20)
add_compile_options(-Wall -Wextra -Weffc++)

add_compile_options(${Boost_CXX_FLAGS})

include_directories("${PROJECT_SOURCE_DIR}" "${Boost_INCLUDE_DIRS}/..")

add_executable(qttst qttst.cpp qttst.h)
target_link_libraries(qttst ${Boost_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets)
