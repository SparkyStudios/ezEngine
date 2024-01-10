ez_cmake_init()

if (EZ_PLATFORM_WINDOWS)
  ez_requires_d3d()
endif ()

ez_requires_assimp()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(APPLICATION ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Core
  GameEngine

  PRIVATE
  RAI
  RHI
  RPI
  Texture
)

if (EZ_CMAKE_PLATFORM_WINDOWS)
  target_link_libraries(${PROJECT_NAME} PRIVATE RHID3D11)
  ez_link_target_dx11(${PROJECT_NAME})
endif()

if (EZ_CMAKE_PLATFORM_OSX)
  target_link_libraries(${PROJECT_NAME} PRIVATE RHIMTL)
endif()

ez_link_target_assimp(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME}
  InspectorPlugin
)
