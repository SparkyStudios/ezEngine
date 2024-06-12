// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <RPI/RPIPCH.h>

#include <RPI/Materials/Functors/MaterialFunctor_isset.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMaterialFunctor_isset, 1, ezRTTIDefaultAllocator<spMaterialFunctor_isset>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spMaterialFunctor_isset::spMaterialFunctor_isset()
    : spMaterialFunctor("isset")
  {
  }

  ezVariant spMaterialFunctor_isset::Evaluate(const ezArrayPtr<ezVariant>& arguments) const
  {
    return arguments.GetCount() == 1 && arguments[0].IsValid();
  }
} // namespace RPI
