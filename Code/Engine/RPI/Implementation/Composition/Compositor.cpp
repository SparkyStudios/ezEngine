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

#include <RPI/RPIPCH.h>

#include <RPI/Composition/Compositor.h>
#include <RPI/Core/Renderer.h>

#include <RHI/Device.h>

namespace RPI
{
  typedef ezRTTIDefaultAllocator<spCompositor, RHI::spDeviceAllocatorWrapper> spRTTICompositorAllocator;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spCompositorEntryPoint, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spCompositor, 1, spRTTICompositorAllocator)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_MEMBER_PROPERTY("GameRenderer", m_pGameRenderer),
      EZ_MEMBER_PROPERTY("EditorRenderer", m_pEditorRenderer),
      EZ_MEMBER_PROPERTY("PreviewRenderer", m_pPreviewRenderer),
    }
    EZ_END_PROPERTIES;
  }
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  ezResult spCompositorEntryPoint::Serialize(ezStreamWriter& inout_stream) const
  {
    inout_stream << m_iPinIndex;
    inout_stream << m_uiNumConnections;
    return EZ_SUCCESS;
  }

  ezResult spCompositorEntryPoint::Deserialize(ezStreamReader& inout_stream)
  {
    inout_stream >> m_iPinIndex;
    inout_stream >> m_uiNumConnections;
    return EZ_SUCCESS;
  }

  spCompositor::spCompositor()
  {
  }
} // namespace RPI
