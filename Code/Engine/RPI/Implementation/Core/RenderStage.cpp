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

#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderStage.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderStageFilter, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderStage, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spRenderStage::spRenderStage(ezStringView sName)
  {
    m_sName.Assign(sName);
    spRenderSystem::GetSingleton()->RegisterRenderStage(this);
  }

  RHI::spOutputDescription spRenderStage::GetOutputDescription(const spRenderView* pRenderView) const
  {
    const ezUInt32 uiIndex = m_RenderViewFramebuffers.Find(pRenderView);
    if (uiIndex == ezInvalidIndex)
      return {};

    return m_RenderViewFramebuffers.GetValue(uiIndex)->GetOutputDescription();
  }

  ezSharedPtr<RHI::spFramebuffer> spRenderStage::GetOutputFramebuffer(const spRenderView* pRenderView) const
  {
    const ezUInt32 uiIndex = m_RenderViewFramebuffers.Find(pRenderView);
    if (uiIndex == ezInvalidIndex)
      return nullptr;

    return m_RenderViewFramebuffers.GetValue(uiIndex);
  }
} // namespace RPI
