cmake_minimum_required(VERSION 3.5.0)
project(ApiSample LANGUAGES C CXX)

aux_source_directory(${PROJECT_SOURCE_DIR} SAMPLE_CXX_FILES)
message("****************************SAMPLE_CXX_FILES: ${SAMPLE_CXX_FILES}")

add_executable(ApiSample ${SAMPLE_CXX_FILES})
target_link_libraries(ApiSample ${FRAMEWORK_NAME})

