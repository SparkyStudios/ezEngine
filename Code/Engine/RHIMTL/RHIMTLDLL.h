#pragma once

#include <Foundation/Basics.h>

#include <RHI/RHIDLL.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RHIMTL_LIB
#    define SP_RHIMTL_DLL EZ_DECL_EXPORT
#  else
#    define SP_RHIMTL_DLL EZ_DECL_IMPORT
#  endif
#else
#  define SP_RHIMTL_DLL
#endif


#define SP_RHI_MTL_RELEASE(mtlObj) \
  do                               \
  {                                \
    if ((mtlObj) != nullptr)       \
    {                              \
      (mtlObj)->release();         \
      (mtlObj) = nullptr;          \
    }                              \
  } while (0)

#define SP_RHI_MTL_RETAIN(mtlObj) \
  do                              \
  {                               \
    if ((mtlObj) != nullptr)      \
    {                             \
      (mtlObj)->retain();         \
    }                             \
  } while (0)
