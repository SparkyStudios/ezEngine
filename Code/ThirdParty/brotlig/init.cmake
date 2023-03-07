# Brotli-G SDK Support

set(EZ_3RDPARTY_BROTLIG_SUPPORT ON CACHE BOOL "Whether support for Brotli-G compression should be enabled")
mark_as_advanced(FORCE EZ_3RDPARTY_BROTLIG_SUPPORT)

macro(ez_requires_brotlig)
  ez_requires_one_of(EZ_CMAKE_PLATFORM_WINDOWS EZ_CMAKE_PLATFORM_LINUX)
  ez_requires(EZ_3RDPARTY_BROTLIG_SUPPORT)

  if(EZ_CMAKE_PLATFORM_WINDOWS_UWP)
    return()
  endif()
endmacro()
