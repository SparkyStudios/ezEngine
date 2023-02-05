#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Basics/Platform/Win/HResultUtils.h>

#include <RHI/RHIDLL.h>

#include <d3d11_3.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
#  include <dxgi1_3.h>
#else
#  include <dxgi.h>
#endif


// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RHID3D11_LIB
#    define SP_RHID3D11_DLL EZ_DECL_EXPORT
#  else
#    define SP_RHID3D11_DLL EZ_DECL_IMPORT
#  endif
#else
#  define SP_RHID3D11_DLL
#endif


#define SP_RHI_DX11_RELEASE(d3dobj) \
  do                                \
  {                                 \
    if ((d3dobj) != nullptr)        \
    {                               \
      (d3dobj)->Release();          \
      (d3dobj) = nullptr;           \
    }                               \
  } while (0)
