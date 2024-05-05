ez_cmake_init()

sp_requires_mikktspace()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

ez_enable_strict_warnings(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Foundation
  Texture
)

sp_link_target_mikktspace(${PROJECT_NAME})

if (EZ_3RDPARTY_LUA_SUPPORT)

  target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Lua
  )

endif()

if (EZ_3RDPARTY_DUKTAPE_SUPPORT)

  target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Duktape
  )

endif()

if(EZ_3RDPARTY_GLFW_SUPPORT)

  target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

endif()
