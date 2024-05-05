// Copyright (c) 2024-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Basics/Platform/Win/HResultUtils.h>
#include <Foundation/Basics/Platform/Win/IncludeWindows.h>

#include <RHI/RHIDLL.h>

#include <d3d12.h>
#include <D3D12MemAlloc.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
#  include <dxgi1_3.h>
#else
#  include <dxgi.h>
#endif


// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RHID3D12_LIB
#    define SP_RHID3D12_DLL EZ_DECL_EXPORT
#  else
#    define SP_RHID3D12_DLL EZ_DECL_IMPORT
#  endif
#else
#  define SP_RHID3D12_DLL
#endif


#define SP_RHI_DX12_RELEASE(d3dobj) \
  do                                \
  {                                 \
    if ((d3dobj) != nullptr)        \
    {                               \
      (d3dobj)->Release();          \
      (d3dobj) = nullptr;           \
    }                               \
  } while (0)
