#pragma once

#include <Foundation/Algorithm/HashableStruct.h>
#include <Foundation/Basics.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Id.h>
#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>


// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RHI_LIB
#    define SP_RHI_DLL EZ_DECL_EXPORT
#  else
#    define SP_RHI_DLL EZ_DECL_IMPORT
#  endif
#else
#  define SP_RHI_DLL
#endif
