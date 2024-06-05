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

#include <RPI/Meshes/MeshRenderObject.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(RPI::spMeshLevelOfDetailFetchFunction, 1)
  EZ_ENUM_CONSTANT(RPI::spMeshLevelOfDetailFetchFunction::Constant),
  EZ_ENUM_CONSTANT(RPI::spMeshLevelOfDetailFetchFunction::Logarithmic),
  EZ_ENUM_CONSTANT(RPI::spMeshLevelOfDetailFetchFunction::Exponential),
EZ_END_STATIC_REFLECTED_ENUM;
// clang-format on

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMeshRenderObject, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  bool spMeshRenderObject::CanBeInstanceOf(spRenderObject* pRenderObject) const
  {
    return pRenderObject->IsInstanceOf<spMeshRenderObject>() && ezStaticCast<spMeshRenderObject*>(pRenderObject)->m_hMeshResource == m_hMeshResource;
  }

  ezResult spMeshRenderObject::Instantiate(spRenderObject* pRenderObject)
  {
    if (!CanBeInstanceOf(pRenderObject))
      return EZ_FAILURE;

    const spMeshRenderObject* pMeshRenderObject = ezStaticCast<spMeshRenderObject*>(pRenderObject);

    spPerInstanceData instance;
    pMeshRenderObject->FillInstanceData(instance);

    if (m_Instances.Contains(pMeshRenderObject->m_uiUniqueID))
    {
      m_PerInstanceData[m_Instances[pMeshRenderObject->m_uiUniqueID]] = instance;
    }
    else
    {
      m_Instances[pMeshRenderObject->m_uiUniqueID] = m_PerInstanceData.GetCount();
      m_PerInstanceData.PushBack(instance);

      m_bIndirectBufferDirty = true;
    }

    return EZ_SUCCESS;
  }

  void spMeshRenderObject::MakeRootInstance()
  {
    m_Instances.Clear();
    m_PerInstanceData.Clear();

    spPerInstanceData instance;
    FillInstanceData(instance);

    m_PerInstanceData.PushBack(instance);
    m_bIndirectBufferDirty = true;
  }

  bool spMeshRenderObject::HasInstances()
  {
    return !m_PerInstanceData.IsEmpty();
  }

  void spMeshRenderObject::FillInstanceData(spPerInstanceData& instance) const
  {
    instance.m_Transform = m_Transform;
    instance.m_PreviousTransform = m_PreviousTransform;

    if (m_Transform.ContainsUniformScale())
    {
      instance.m_NormalTransform = instance.m_Transform;
    }
    else
    {
      const ezMat4 worldMatrix = m_Transform.GetAsMat4();
      const ezMat3 normalMatrix = worldMatrix.GetRotationalPart().GetInverse(0.0f);
      instance.m_NormalTransform = normalMatrix.GetTranspose();
    }
  }

  ezUInt32 spMeshRenderObject::GetBufferSize() const
  {
    return m_PerInstanceData.GetCount() * sizeof(spPerInstanceData);
  }

  void spMeshRenderObject::CreateBuffer()
  {
    RHI::spBufferDescription desc;
    desc.m_eUsage = RHI::spBufferUsage::StructuredBufferReadOnly | RHI::spBufferUsage::Dynamic;
    desc.m_uiSize = GetBufferSize();
    desc.m_uiStructureStride = sizeof(spPerInstanceData);

    m_pPerInstanceDataBuffer = spRenderSystem::GetSingleton()->GetDevice()->GetResourceFactory()->CreateBuffer(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    m_pPerInstanceDataBuffer->SetDebugName("Buffer_PerInstance");
#endif
  }

  void spMeshRenderObject::UpdateBuffer()
  {
    if (m_PerInstanceData.IsEmpty())
      return;

    if (m_pPerInstanceDataBuffer == nullptr)
      CreateBuffer();

    if (m_pPerInstanceDataBuffer->GetSize() < GetBufferSize())
    {
      m_pPerInstanceDataBuffer.Clear();
      CreateBuffer();
    }

    RHI::spDevice* pDevice = spRenderSystem::GetSingleton()->GetDevice();
    pDevice->UpdateBuffer(m_pPerInstanceDataBuffer, 0, m_PerInstanceData.GetArrayPtr());

    if (m_bIndirectBufferDirty)
    {
      const ezResourceLock resource(m_hMeshResource, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (!resource.IsValid())
        return; // ezLog::Error("Unable to get the mesh resource!");

      const auto& mesh = resource.GetPointerNonConst()->GetLOD(0);

      m_DrawCommands.Clear();
      mesh.GetDrawCommands(m_DrawCommands, m_PerInstanceData.GetCount());

      if (const ezUInt32 uiBufferSize = pDevice->GetIndexedIndirectCommandSize() * m_DrawCommands.GetCount(); m_pIndirectBuffer == nullptr || m_pIndirectBuffer->GetSize() < uiBufferSize)
        m_pIndirectBuffer = pDevice->GetResourceFactory()->CreateBuffer(RHI::spBufferDescription(uiBufferSize, RHI::spBufferUsage::IndirectBuffer));

      pDevice->UpdateIndexedIndirectBuffer(m_pIndirectBuffer, m_DrawCommands.GetArrayPtr());

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      ezStringBuilder sb;
      sb.SetFormat("{0}__IndirectBuffer", mesh.GetRootNode().m_sName);
      m_pIndirectBuffer->SetDebugName(sb);
#endif

      m_bIndirectBufferDirty = false;
    }
  }
} // namespace RPI
