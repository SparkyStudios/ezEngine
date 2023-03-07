ez_cmake_init()

ez_requires_brotlig()

set(PROJECT_NAME "brotli")

file(GLOB COMMON c/common/*.c c/common/*.h c/include/brotli/port.h c/include/brotli/types.h)
file(GLOB ENCODER c/enc/*.c c/enc/*.h c/include/brotli/encode.h)
file(GLOB DECODER c/dec/*.c c/dec/*.h c/include/brotli/decoder.h)

ez_create_target(
  STATIC_LIBRARY ${PROJECT_NAME}
  NO_UNITY
  NO_EZ_PREFIX
  NO_WARNINGS_AS_ERRORS
  MANUAL_SOURCE_FILES ${COMMON} ${ENCODER} ${DECODER}
)

# override the default IDE folder
ez_set_project_ide_folder(${PROJECT_NAME} "Code/ThirdParty/brotli")

target_compile_features(${PROJECT_NAME} PUBLIC)

include_directories("c/include")
include_directories("c/common")
include_directories("c/enc")
include_directories("c/dec")

source_group("Common" FILES ${COMMON})
source_group("Decoder" FILES ${DECODER})
source_group("Encoder" FILES ${ENCODER})

list(APPEND INCLUDE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/c/include")
list(APPEND INCLUDE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/c/common")
list(APPEND INCLUDE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/c/include")
list(APPEND INCLUDE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/c/include")

target_include_directories(${PROJECT_NAME} INTERFACE
  "$<BUILD_INTERFACE:${includePath}>"
)
