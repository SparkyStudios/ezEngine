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

#include <RPI/RPIDLL.h>

#include <RPI/Composition/CameraSlot.h>
#include <RPI/Core/RenderSystem.h>

namespace RPI
{
  class SP_RPI_DLL spCompositor : public ezRefCounted
  {
    EZ_DISALLOW_COPY_AND_ASSIGN(spCompositor);

  public:
    spCompositor();
    ~spCompositor() override = default;

  private:
    ezUniquePtr<spRenderSystem> m_pRenderSystem{nullptr};
    ezHybridArray<spCameraSlot, 8> m_Cameras;
  };
} // namespace RPI
