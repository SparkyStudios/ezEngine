// Copyright (c) 2023-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <Foundation/Algorithm/HashableStruct.h>
#include <Foundation/Basics.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Math/Size.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Id.h>
#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>


// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RAI_LIB
#    define SP_RAI_DLL EZ_DECL_EXPORT
#  else
#    define SP_RAI_DLL EZ_DECL_IMPORT
#  endif
#else
#  define SP_RAI_DLL
#endif
