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

namespace RPI
{
  class spRenderSystem;
  class spRenderContext;
  class spRenderFeature;

  class SP_RPI_DLL spCompositorEntryPoint : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spCompositorEntryPoint, ezReflectedClass);

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsConnected() const
    {
      return m_iPinIndex != -1;
    }

    ezResult Serialize(ezStreamWriter& inout_stream) const;
    ezResult Deserialize(ezStreamReader& inout_stream);

  protected:
    ezInt16 m_iPinIndex{-1};
    ezUInt8 m_uiNumConnections{0};
  };

  class SP_RPI_DLL spCompositor : public ezReflectedClass, public ezRefCounted
  {
    EZ_ADD_DYNAMIC_REFLECTION(spCompositor, ezReflectedClass);
    EZ_DISALLOW_COPY_AND_ASSIGN(spCompositor);

  public:
    spCompositor();
    ~spCompositor() override = default;

    void Render(const spRenderContext* pRenderContext);

  private:
    spRenderSystem* m_pRenderSystem{nullptr};

    ezDynamicArray<spCameraSlot*> m_Cameras;
    ezDynamicArray<spRenderFeature*> m_RenderFeatures;

    spCompositorEntryPoint m_GameEntryPoint;
    spCompositorEntryPoint m_EditorEntryPoint;
    spCompositorEntryPoint m_PreviewEntryPoint;
  };
} // namespace RPI
