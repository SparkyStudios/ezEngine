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

#include <RPI/RPIDLL.h>

#include <RPI/Materials/MaterialData.h>

#include <RPI/Resources/ShaderResource.h>

#include <Core/World/World.h>

namespace RPI
{
  typedef ezComponentManagerSimple<class spMaterialComponent, ezComponentUpdateType::WhenSimulating, ezBlockStorageType::Compact> spMaterialComponentManager;

  class SP_RPI_DLL spMaterialComponent : public ezComponent
  {
    EZ_DECLARE_COMPONENT_TYPE(spMaterialComponent, ezComponent, spMaterialComponentManager);

  private:
  };
} // namespace RPI
