ez_cmake_init()

sp_requires_ktx()
ez_requires_assimp()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Core
  Texture
  RHI

  PRIVATE
  meshoptimizer
  mikktspace
  ozz
)

sp_link_target_ktx(${PROJECT_NAME})
ez_link_target_assimp(${PROJECT_NAME})
