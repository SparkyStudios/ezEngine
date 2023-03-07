ez_cmake_init()

set(CMAKE_CXX_STANDARD 17)

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

add_subdirectory("external/brotli")
include_directories("external/brotli/c/include")
include_directories("external")

file(GLOB COMMON src/common/*.cpp src/DataStream.cpp inc/common/*.h inc/DataStream.h)
file(GLOB ENCODER src/encoder/*.cpp src/BrotligEncoder.cpp inc/encoder/*.h inc/BrotligEncoder.h)
file(GLOB DECODER src/decoder/*.cpp src/BrotligDecoder.cpp inc/decoder/*.h inc/BrotligDecoder.h)

ez_create_target(
  STATIC_LIBRARY ${PROJECT_NAME}
  NO_UNITY
  NO_EZ_PREFIX
  NO_WARNINGS_AS_ERRORS
  MANUAL_SOURCE_FILES inc/BrotliG.h ${COMMON} ${ENCODER} ${DECODER}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PUBLIC BUILDSYSTEM_ENABLE_BROTLIG_SUPPORT)

include_directories("inc")
include_directories("inc/common")
include_directories("inc/decoder")
include_directories("inc/encoder")

source_group("Common" FILES ${COMMON})
source_group("Decoder" FILES ${DECODER})
source_group("Encoder" FILES ${ENCODER})

target_include_directories(${PROJECT_NAME} INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_include_directories(${PROJECT_NAME} INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/external
)

target_include_directories(${PROJECT_NAME} INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/external/brotli/c/include
)

target_link_libraries(${PROJECT_NAME} brotli)
