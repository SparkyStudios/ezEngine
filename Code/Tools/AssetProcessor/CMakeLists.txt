ez_cmake_init()

sp_requires_ktx()
ez_requires_assimp()
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(APPLICATION ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  Foundation
  RAI

  meshoptimizer
  mikktspace
  ozz
)

ez_link_target_assimp(${PROJECT_NAME})
sp_link_target_ktx(${PROJECT_NAME})
