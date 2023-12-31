cmake_minimum_required(VERSION 3.5.0)
project(LearnVulkan VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3rd party
set(GLFW_INC_DIR ${PROJECT_SOURCE_DIR}/glfw/include)
set(GLFW_LIB_DIR ${PROJECT_SOURCE_DIR}/glfw/lib-vc2022)
set(VULKAN_INC_DIR ${PROJECT_SOURCE_DIR}/vulkan/Include)
set(VULKAN_LIB_DIR ${PROJECT_SOURCE_DIR}/vulkan/Lib)

# project src
aux_source_directory(${PROJECT_SOURCE_DIR}/src/core CORE_CXX_FILES)
message("****************************CORE_CXX_FILES: ${CORE_CXX_FILES}")
aux_source_directory(${PROJECT_SOURCE_DIR} DEMO_CXX_FILES)
message("****************************DEMO_CXX_FILES: ${DEMO_CXX_FILES}")

add_executable(${PROJECT_NAME} ${CORE_CXX_FILES} ${DEMO_CXX_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${GLFW_INC_DIR} ${VULKAN_INC_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE ${GLFW_LIB_DIR} ${VULKAN_LIB_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE glfw3.lib vulkan-1.lib)

