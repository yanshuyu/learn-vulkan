cmake_minimum_required(VERSION 3.5.0)
project(${FRAMEWORK_NAME} LANGUAGES C CXX)

# 3rd party
set(3PARTY_DIR ${LEARN_VK_ROOT_DIR}/3party)
set(GLFW_INC_DIR ${3PARTY_DIR}/glfw/include)
#set(GLFW_LIB_DIR ${3PARTY_DIR}/glfw/lib-vc2022)
set(GLM_INC_DIR ${3PARTY_DIR}/glm)
set(VULKAN_INC_DIR ${3PARTY_DIR}/vulkan/Include)
set(VULKAN_LIB_DIR ${3PARTY_DIR}/vulkan/Lib)


#include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${GLFW_INC_DIR} ${VULKAN_INC_DIR})
#link_directories(${GLFW_LIB_DIR} ${VULKAN_LIB_DIR})

# framework src
#[[
message("${FRAMEWORK_NAME} Directry ------------>>>>>>>>>>> ${PROJECT_SOURCE_DIR}")
aux_source_directory(${3PARTY_DIR}/stb STB_CXX_FILES)
message("****************************STB_CXX_FILES: ${STB_CXX_FILES}")
aux_source_directory(${PROJECT_SOURCE_DIR}/core CORE_CXX_FILES)
message("****************************CORE_CXX_FILES: ${CORE_CXX_FILES}")
aux_source_directory(${PROJECT_SOURCE_DIR}/rendering RENDERING_CXX_FILES)
message("****************************RENDERING_CXX_FILES: ${RENDERING_CXX_FILES}")
aux_source_directory(${PROJECT_SOURCE_DIR}/input INPUT_CXX_FILES)
message("****************************INPUT_CXX_FILES: ${INPUT_CXX_FILES}")
aux_source_directory(${PROJECT_SOURCE_DIR}/app APP_CXX_FILES)
message("****************************APP_CXX_FILES: ${APP_CXX_FILES}")
]]

file(GLOB_RECURSE FRAMEWORK_SRC "*.cpp" "*.c" "*.h" "*.hpp")
message("FRAMEWORK_SRC --> ${FRAMEWORK_SRC}")
# framework lib
add_library(${FRAMEWORK_NAME} STATIC ${FRAMEWORK_SRC})
target_include_directories(${FRAMEWORK_NAME} PUBLIC ${LEARN_VK_ROOT_DIR}
                                                    ${3PARTY_DIR}  
                                                    ${PROJECT_SOURCE_DIR} 
                                                    ${GLM_INC_DIR}
                                                    ${GLFW_INC_DIR} 
                                                    ${VULKAN_INC_DIR})
target_link_directories(${FRAMEWORK_NAME} PUBLIC  ${VULKAN_LIB_DIR})

target_link_libraries(${FRAMEWORK_NAME} PUBLIC glm glfw glslang SPIRV glslang-default-resource-limits vulkan-1.lib)

