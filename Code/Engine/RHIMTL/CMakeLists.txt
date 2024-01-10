ez_cmake_init()

ez_requires_osx()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Foundation
  RHI
  "-framework Foundation"
  "-framework Metal"
  "-framework QuartzCore"
)

set(THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/${EZ_SUBMODULE_PREFIX_PATH}/Code/ThirdParty")
target_include_directories(${PROJECT_NAME} PUBLIC "${THIRDPARTY_DIR}/metal-cpp")

find_library(METAL Metal)
target_include_directories(${PROJECT_NAME} PUBLIC ${METAL}/Headers)
