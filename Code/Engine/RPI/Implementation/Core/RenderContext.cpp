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

#include <RPI/Core/RenderContext.h>
#include <RPI/Core/RenderSystem.h>

using namespace RHI;

namespace RPI
{
  spRenderContext::spRenderContext(spDevice* pDevice)
    : m_pDevice(pDevice)
    , m_Data()
  {
  }

  spRenderContext::~spRenderContext()
  {
    m_pCommandList.Clear();
  }

  void spRenderContext::SetCommandList(ezSharedPtr<spCommandList> pCommandList)
  {
    if (m_pCommandList == pCommandList)
      return;

    m_pCommandList = pCommandList;
    m_pCommandList->Reset();
  }

  void spRenderContext::BeginFrame()
  {
    m_pCommandList->Begin();
  }

  void spRenderContext::EndFrame(ezSharedPtr<spFence> pFence)
  {
    m_pCommandList->End();

    m_pDevice->SubmitCommandList(m_pCommandList, pFence);
  }

  void spRenderContext::Reset()
  {
    m_pCommandList->Reset();
  }

  const spRenderContextData& spRenderContext::GetExtractionData() const
  {
    return m_Data[spRenderSystem::GetDataIndexForExtraction()];
  }

  spRenderContextData& spRenderContext::GetExtractionData()
  {
    return m_Data[spRenderSystem::GetDataIndexForExtraction()];
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Core_RenderContext);
