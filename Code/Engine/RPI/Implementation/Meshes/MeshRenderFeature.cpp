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

#include <RPI/Meshes/MeshRenderFeature.h>
#include <RPI/Meshes/MeshRenderFeatureExtractor.h>
#include <RPI/Meshes/MeshRenderObject.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMeshRenderFeature, 1, ezRTTIDefaultAllocator<spMeshRenderFeature>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spMeshRenderFeature::GetSupportedRenderObjectTypes(ezHybridArray<const ezRTTI*, 8>& out_Types) const
  {
    out_Types.SetCount(1);
    out_Types.PushBack(ezGetStaticRTTI<spMeshRenderObject>());
  }

  void spMeshRenderFeature::Draw(spRenderObject* pRenderObject, const spRenderContext* pRenderingContext)
  {
    const auto cl = pRenderingContext->GetCommandList();
    const auto pMeshRenderObject = ezStaticCast<spMeshRenderObject*>(pRenderObject);

    const spRenderView* pRenderView = pRenderingContext->GetExtractionData().m_pRenderView;
    const spRenderStage* pRenderStage = pRenderingContext->GetExtractionData().m_pRenderStage;

    const ezResourceLock resource(pMeshRenderObject->m_hMeshResource, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (!resource.IsValid())
      return; // ezLog::Error("Unable to get the mesh resource!");

    // Make sure the instances buffer is updated.
    pMeshRenderObject->UpdateBuffer();

    const ezUInt32 uiLODCount = resource.GetPointer()->GetDescriptor().GetNumLODs();

    ezUInt32 uiLOD = 0;

    if (uiLODCount > 1)
    {
      if (ezMath::IsEqual(pMeshRenderObject->m_fLODMaxDistance, 0.0f, ezMath::SmallEpsilon<float>()))
      {
        uiLOD = uiLODCount - 1;
      }
      else
      {
        const float fDistanceToCamera = ezMath::Floor((pRenderView->GetPosition() - pMeshRenderObject->m_Transform.m_vPosition).GetLengthSquared());
        const float fMaxDistance = ezMath::Square(pMeshRenderObject->m_fLODMaxDistance);

        // Constant LOD fetch function
        if (pMeshRenderObject->m_eLODFetchFunction == spMeshLevelOfDetailFetchFunction::Constant)
        {
          const float fSteps = fMaxDistance / static_cast<float>(uiLODCount);

          for (ezInt32 i = uiLODCount - 1; i >= 0; i--)
          {
            if (fDistanceToCamera < (i + 1) * fSteps)
              continue;

            uiLOD = i;
            break;
          }
        }
        // Logarithmic LOD fetch function
        else if (pMeshRenderObject->m_eLODFetchFunction == spMeshLevelOfDetailFetchFunction::Logarithmic)
        {
          float fCurrentMaxDistance = fMaxDistance;

          for (ezInt32 i = uiLODCount - 1; i >= 0; i--)
          {
            fCurrentMaxDistance -= fMaxDistance / static_cast<float>(ezMath::Pow2(i + 1));

            if (fDistanceToCamera < fCurrentMaxDistance)
              continue;

            uiLOD = i;
            break;
          }
        }
        // Exponential LOD fetch function
        else if (pMeshRenderObject->m_eLODFetchFunction == spMeshLevelOfDetailFetchFunction::Exponential)
        {
          float fCurrentMaxDistance = fMaxDistance;

          for (ezInt32 i = uiLODCount - 1; i >= 0; i--)
          {
            fCurrentMaxDistance -= fMaxDistance / ezMath::Pow2(static_cast<ezInt32>(uiLODCount - i));

            if (fDistanceToCamera < fCurrentMaxDistance)
              continue;

            uiLOD = i;
            break;
          }
        }
      }
    }

    auto& mesh = resource.GetPointerNonConst()->GetLOD(uiLOD);

    mesh.CreateRHIVertexBuffer();
    mesh.CreateRHIIndexBuffer();
    mesh.CreateRHIInputLayout();

    auto* pShaderManager = ezSingletonRegistry::GetRequiredSingletonInstance<spShaderManager>();

    {
      spShaderCompilerSetup setup;
      setup.m_hMaterialResource = pMeshRenderObject->m_hMaterialResource;
      setup.m_hShaderResource = m_hShader;
      setup.m_eStage = RHI::spShaderStage::PixelShader;
      setup.m_PredefinedMacros.PushBack({"SP_FEATURE_VERTEX_SKINNING", "SP_OFF"});

      m_pPixelShader = pShaderManager->CompileShader(setup);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pPixelShader->SetDebugName("MeshRenderFeature_PixelShader");
#endif
    }

    {
      m_pShaderProgram = cl->GetDevice()->GetResourceFactory()->CreateShaderProgram();
      m_pShaderProgram->Attach(m_pVertexShader);
      m_pShaderProgram->Attach(m_pPixelShader);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pShaderProgram->SetDebugName("MeshRenderFeature_ShaderProgram");
#endif
    }

    {
      RHI::spResourceSetDescription desc{};
      desc.m_hResourceLayout = m_pResourceLayout->GetHandle();
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerFrame"), pRenderingContext->GetFrameDataBuffer().GetHandle());
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerView"), pRenderView->GetDataBuffer().GetHandle());
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerInstance"), pMeshRenderObject->m_pPerInstanceDataBuffer->GetHandle());
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerMaterial"), pMeshRenderObject->m_pPerMaterialDataBuffer->GetHandle());

      m_pResourceSet = cl->GetDevice()->GetResourceFactory()->CreateResourceSet(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pResourceSet->SetDebugName("MeshRenderFeature_ResourceSet_0");
#endif
    }

    {
      RHI::spGraphicPipelineDescription desc{};
      desc.m_bSupportsPushConstants = false;
      desc.m_Output = pRenderStage->GetOutputDescription(pRenderView);
      desc.m_ePrimitiveTopology = RHI::spPrimitiveTopology::Triangles;
      desc.m_RenderingState = pRenderStage->GetRenderingState(pRenderObject);
      desc.m_ShaderPipeline.m_hShaderProgram = m_pShaderProgram->GetHandle();
      desc.m_ShaderPipeline.m_InputLayouts.PushBack(mesh.GetRHIInputLayout()->GetHandle());
      desc.m_ResourceLayouts.PushBack(m_pResourceLayout->GetHandle());

      if (m_pGraphicPipeline == nullptr || m_pGraphicPipeline->GetDescription().CalculateHash() == desc.CalculateHash())
      {
        m_pGraphicPipeline = cl->GetDevice()->GetResourceFactory()->CreateGraphicPipeline(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
        m_pGraphicPipeline->SetDebugName("MeshRenderFeature_GraphicPipeline");
#endif
      }
    }

    cl->PushDebugGroup("spMeshRenderFeature");
    {
      cl->SetGraphicPipeline(m_pGraphicPipeline);

      cl->SetGraphicResourceSet(0, m_pResourceSet);

      cl->SetVertexBuffer(0, mesh.GetRHIVertexBuffer());
      cl->SetIndexBuffer(mesh.GetRHIIndexBuffer(), RHI::spIndexFormat::UInt16);

      cl->DrawIndexedIndirect(pMeshRenderObject->m_pIndirectBuffer, 0, pMeshRenderObject->m_DrawCommands.GetCount(), cl->GetDevice()->GetIndexedIndirectCommandSize());
    }
    cl->PopDebugGroup();
  }

  spMeshRenderFeature::spMeshRenderFeature()
    : spRenderFeature(EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spMeshRenderFeatureExtractor))
  {
    m_hShader = ezResourceManager::LoadResource<RAI::spShaderResource>(":shaders/RPI/RenderStages/OpaqueRenderStage.slang");

    const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<spRenderSystem>()->GetDevice();
    auto* pShaderManager = ezSingletonRegistry::GetRequiredSingletonInstance<spShaderManager>();

    {
      spShaderCompilerSetup setup;
      setup.m_hShaderResource = m_hShader;
      setup.m_eStage = RHI::spShaderStage::VertexShader;
      setup.m_PredefinedMacros.PushBack({"SP_FEATURE_VERTEX_SKINNING", "SP_OFF"});

      m_pVertexShader = pShaderManager->CompileShader(setup);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pVertexShader->SetDebugName("MeshRenderFeature_VertexShader");
#endif
    }

    {
      RHI::spResourceLayoutDescription desc{};
      desc.m_eShaderStage = RHI::spShaderStage::VertexShader | RHI::spShaderStage::PixelShader;

      RHI::spResourceLayoutElementDescription perFrameBuffer{};
      perFrameBuffer.m_sName = ezMakeHashedString("Buffer_PerFrame");
      perFrameBuffer.m_eType = RHI::spShaderResourceType::ConstantBuffer;
      perFrameBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      RHI::spResourceLayoutElementDescription perViewBuffer{};
      perViewBuffer.m_sName = ezMakeHashedString("Buffer_PerView");
      perViewBuffer.m_eType = RHI::spShaderResourceType::ConstantBuffer;
      perViewBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      RHI::spResourceLayoutElementDescription perInstanceBuffer{};
      perInstanceBuffer.m_sName = ezMakeHashedString("Buffer_PerInstance");
      perInstanceBuffer.m_eType = RHI::spShaderResourceType::ReadOnlyStructuredBuffer;
      perInstanceBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      RHI::spResourceLayoutElementDescription perMaterialBuffer{};
      perMaterialBuffer.m_sName = ezMakeHashedString("Buffer_PerMaterial");
      perMaterialBuffer.m_eType = RHI::spShaderResourceType::ReadOnlyStructuredBuffer;
      perMaterialBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      desc.m_Elements.PushBack(perFrameBuffer);
      desc.m_Elements.PushBack(perViewBuffer);
      desc.m_Elements.PushBack(perInstanceBuffer);
      desc.m_Elements.PushBack(perMaterialBuffer);

      m_pResourceLayout = pDevice->GetResourceFactory()->CreateResourceLayout(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pResourceLayout->SetDebugName("MeshRenderFeature_ResourceLayout");
#endif
    }
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshRenderFeature);
