ez_cmake_init()

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(APPLICATION ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  Foundation
  RAI

  avir
  meshoptimizer
  mikktspace
  rgbcx
  ozz
)

ez_link_target_brotlig(${PROJECT_NAME})
ez_link_target_assimp(${PROJECT_NAME})
