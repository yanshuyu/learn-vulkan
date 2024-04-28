cmake_minimum_required(VERSION 3.5.0)
project(LEARN_VULKAN)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FRAMEWORK_NAME "learnvk-framework")

set(LEARN_VK_ROOT_DIR ${PROJECT_SOURCE_DIR})
message("LEARN_VK_ROOT_DIR -------->>>>>>>>>>> ${PROJECT_SOURCE_DIR}")

add_definitions(-DASSETS_DIR=\"${PROJECT_SOURCE_DIR}/assets/\")

#include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory("src")
add_subdirectory("sample")
