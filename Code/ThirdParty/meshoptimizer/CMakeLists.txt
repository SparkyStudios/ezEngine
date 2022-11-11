ez_cmake_init()



# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

set(SOURCES
    src/meshoptimizer.h
    src/allocator.cpp
    src/clusterizer.cpp
    src/indexcodec.cpp
    src/indexgenerator.cpp
    src/overdrawanalyzer.cpp
    src/overdrawoptimizer.cpp
    src/simplifier.cpp
    src/spatialorder.cpp
    src/stripifier.cpp
    src/vcacheanalyzer.cpp
    src/vcacheoptimizer.cpp
    src/vertexcodec.cpp
    src/vertexfilter.cpp
    src/vfetchanalyzer.cpp
    src/vfetchoptimizer.cpp
)

ez_create_target(STATIC_LIBRARY ${PROJECT_NAME} NO_EZ_PREFIX MANUAL_SOURCE_FILES ${SOURCES})

target_include_directories(${PROJECT_NAME} INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")

if(EZ_CMAKE_COMPILER_MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wshadow -Wno-missing-field-initializers)
endif()
