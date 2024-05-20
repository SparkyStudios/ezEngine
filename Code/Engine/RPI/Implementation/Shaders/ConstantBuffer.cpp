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

#include <RPI/Core/RenderSystem.h>
#include <RPI/Shaders/ConstantBuffer.h>

#include <RHI/Device.h>

using namespace RHI;

namespace RPI
{
  spConstantBufferBase::spConstantBufferBase(ezUInt32 uiSizeInBytes, spBufferUsage::Enum eUsage)
  {
    const spBufferDescription desc(uiSizeInBytes, spBufferUsage::ConstantBuffer | eUsage);
    m_pBuffer = spRenderSystem::GetSingleton()->GetDevice()->GetResourceFactory()->CreateBuffer(desc);
  }

  spConstantBufferBase::~spConstantBufferBase()
  {
    m_pBuffer.Clear();
  }

  const spMappedResource& spConstantBufferBase::Map(bool bReadOnly) const
  {
    return m_pBuffer->GetDevice()->Map(m_pBuffer, bReadOnly ? spMapAccess::Read : spMapAccess::Write, 0);
  }

  void spConstantBufferBase::Unmap() const
  {
    m_pBuffer->GetDevice()->UnMap(m_pBuffer, 0);
  }

  void spConstantBufferBase::Swap() const
  {
    m_pBuffer->SwapBuffers();
  }

  void spConstantBufferBase::UpdateBuffer(ezUInt32 uiOffsetInBytes, ezUInt32 uiSizeInBytes, const void* pData) const
  {
    m_pBuffer->GetDevice()->UpdateBuffer(m_pBuffer, uiOffsetInBytes, pData, uiSizeInBytes);
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Shaders_ConstantBuffer);
