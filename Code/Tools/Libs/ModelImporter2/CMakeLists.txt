ez_cmake_init()

ez_requires_assimp()
sp_requires_meshoptimizer()
sp_requires_mikktspace()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Foundation
  RendererCore

  PRIVATE
  ozz
)

ez_link_target_assimp(${PROJECT_NAME})
sp_link_target_meshoptimizer(${PROJECT_NAME})
sp_link_target_mikktspace(${PROJECT_NAME})
