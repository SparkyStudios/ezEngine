# #####################################
# ## RHI Utilities
# #####################################

# #####################################
# ## sp_rhi_requires_d3d11()
# #####################################
macro(sp_rhi_requires_d3d11)
  ez_requires_windows()
endmacro()

# #####################################
# ## sp_rhi_requires_d3d12()
# #####################################
macro(sp_rhi_requires_d3d12)
  ez_requires_windows()
endmacro()

# #####################################
# ## sp_rhi_requires_metal()
# #####################################
macro(sp_rhi_requires_metal)
  ez_requires_osx()
endmacro()

# #####################################
# ## sp_rhi_link_target_d3d11(<target>)
# #####################################
function(sp_rhi_link_target_d3d11 TARGET_NAME)
  sp_rhi_requires_d3d11()

  target_link_libraries(${TARGET_NAME} PRIVATE RHID3D11)
  ez_link_target_dx11(${TARGET_NAME})
endfunction()

# #####################################
# ## sp_rhi_link_target_d3d12(<target>)
# #####################################
function(sp_rhi_link_target_d3d12 TARGET_NAME)
  sp_rhi_requires_d3d12()

  target_link_libraries(${TARGET_NAME} PRIVATE RHID3D12)
  sp_link_target_d3d12(${TARGET_NAME})
endfunction()

# #####################################
# ## sp_rhi_link_target_metal(<target>)
# #####################################
function(sp_rhi_link_target_metal TARGET_NAME)
  sp_rhi_requires_metal()

  find_library(METAL Metal REQUIRED)

  set(METAL_CPP_DIR "${CMAKE_SOURCE_DIR}/${EZ_SUBMODULE_PREFIX_PATH}/Code/ThirdParty")
  target_include_directories(${TARGET_NAME} PRIVATE "${METAL_CPP_DIR}/metal-cpp" ${METAL}/Headers)

  target_link_libraries(${TARGET_NAME} PUBLIC "-framework Foundation" "-framework Metal" "-framework QuartzCore")
  target_link_libraries(${TARGET_NAME} PRIVATE RHIMTL)
endfunction()

# #####################################
# ## sp_rhi_requires_renderer()
# #####################################
macro(sp_rhi_requires_renderer)
  if(EZ_CMAKE_PLATFORM_WINDOWS)
    sp_rhi_requires_d3d11()
  elseif(EZ_CMAKE_PLATFORM_OSX)
    sp_rhi_requires_metal()
  endif()
endmacro()

# #####################################
# ## sp_rhi_link_target(<target>)
# ## Add all required libraries and dependencies to the given target so it has access to all available RHI backends.
# #####################################
function(sp_rhi_link_target TARGET_NAME)
  target_link_libraries(${TARGET_NAME} PRIVATE RHI)

  if(EZ_CMAKE_PLATFORM_WINDOWS)
    sp_rhi_link_target_d3d11(${TARGET_NAME})
  elseif(EZ_CMAKE_PLATFORM_OSX)
    sp_rhi_link_target_metal(${TARGET_NAME})
  endif()
endfunction()
