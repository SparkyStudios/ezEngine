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

#include <RAI/RAIPCH.h>

#include <RAI/ShaderVariant.h>

#include <RHI/Device.h>

namespace RAI
{
  spShaderVariant::spShaderVariant(ezStringView sName)
  {
    m_sName.Assign(sName);
  }

  spShaderVariant::~spShaderVariant() noexcept
  {
    Clear();
  }

  void spShaderVariant::Clear()
  {
    m_sName.Clear();
    m_Buffer.Clear();
    m_InputElements.Clear();
    m_EntryPoints.Clear();

    m_pRHIShaderProgram.Clear();
  }

  void spShaderVariant::CreateRHIShaderProgram()
  {
    if (m_pRHIShaderProgram != nullptr)
      return;

    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    m_pRHIShaderProgram = pDevice->GetResourceFactory()->CreateShaderProgram();

    for (const auto& pair : m_EntryPoints)
    {
      ezEnum<RHI::spShaderStage> eStage = pair.key;

      RHI::spShaderDescription desc;
      if (GetRHIShaderDescription(eStage, desc).Failed())
        continue;

      const auto& pShader = pDevice->GetResourceFactory()->CreateShader(desc);
      if (pShader == nullptr)
        continue;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      ezStringBuilder sb;
      sb.Format("{0}__{1}", m_sName, eStage.ToString(eStage));
      pShader->SetDebugName(sb);
#endif

      m_pRHIShaderProgram->Attach(pShader);
    }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.Format("{0}__ShaderProgram", m_sName);
    m_pRHIShaderProgram->SetDebugName(sb);
#endif
  }

  ezResult spShaderVariant::GetRHIShaderDescription(const ezEnum<RHI::spShaderStage>& eStage, RHI::spShaderDescription& out_description) const
  {
    ezUInt32 uiIndex = m_EntryPoints.Find(eStage);
    if (uiIndex == ezInvalidIndex)
      return EZ_FAILURE;

    out_description.m_sEntryPoint = m_EntryPoints.GetValue(uiIndex);
    out_description.m_Buffer = m_Buffer;
    out_description.m_eShaderStage = eStage;

    return EZ_SUCCESS;
  }
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_ShaderVariant)
