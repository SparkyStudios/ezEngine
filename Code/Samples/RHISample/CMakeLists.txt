ez_cmake_init()

sp_rhi_requires_renderer()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(APPLICATION ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Core
  GameEngine

  PRIVATE
  RPI
  Texture
)

sp_rhi_link_target(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME}
  InspectorPlugin
)
