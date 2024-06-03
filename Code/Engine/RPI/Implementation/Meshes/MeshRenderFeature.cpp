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

    const ezResourceLock resource(pMeshRenderObject->m_hMeshResource, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (!resource.IsValid())
      return; // ezLog::Error("Unable to get the mesh resource!");

    // Make sure the instances buffer is updated.
    pMeshRenderObject->UpdateBuffer();

    auto& mesh = resource.GetPointerNonConst()->GetLOD(0);

    mesh.CreateRHIVertexBuffer();
    mesh.CreateRHIIndexBuffer();
    mesh.CreateRHIInputLayout();

    const spRenderView* pRenderView = pRenderingContext->GetExtractionData().m_pRenderView;
    const spRenderStage* pRenderStage = pRenderingContext->GetExtractionData().m_pRenderStage;

    {
      RHI::spResourceSetDescription desc{};
      desc.m_hResourceLayout = m_pResourceLayout->GetHandle();
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerFrame"), pRenderingContext->GetFrameDataBuffer().GetHandle());
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerView"), pRenderView->GetDataBuffer().GetHandle());
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerInstance"), pMeshRenderObject->m_pPerInstanceDataBuffer->GetHandle());

      m_pResourceSet = cl->GetDevice()->GetResourceFactory()->CreateResourceSet(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pResourceSet->SetDebugName("MeshRenderFeature_ResourceSet_0");
#endif
    }

    {
      RHI::spGraphicPipelineDescription desc{};
      desc.m_bSupportsPushConstants = true;
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

      m_PushConstants.transform = pMeshRenderObject->m_Transform.GetAsMat4().GetTranspose();
      m_PushConstants.values = pMeshRenderObject->m_PreviousTransform.GetAsMat4().GetTranspose();
      cl->PushConstants(RHI::spShaderStage::VertexShader, &m_PushConstants, 0, sizeof(PushConstantTest));

      cl->DrawIndexedIndirect(pMeshRenderObject->m_pIndirectBuffer, 0, pMeshRenderObject->m_DrawCommands.GetCount(), cl->GetDevice()->GetIndexedIndirectCommandSize());
    }
    cl->PopDebugGroup();
  }

  spMeshRenderFeature::spMeshRenderFeature()
    : spRenderFeature(EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spMeshRenderFeatureExtractor))
  {
    m_hShader = ezResourceManager::LoadResource<RAI::spShaderResource>(":shaders/point.slang");

    auto* pDevice = spRenderSystem::GetSingleton()->GetDevice();

    {
      spShaderCompilerSetup setup;
      setup.m_eStage = RHI::spShaderStage::VertexShader;
      setup.m_PredefinedMacros.PushBack({"SP_FEATURE_VERTEX_SKINNING", "SP_OFF"});

      m_pVertexShader = spShaderManager::GetSingleton()->CompileShader(m_hShader, setup);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pVertexShader->SetDebugName("MeshRenderFeature_VertexShader");
#endif
    }

    {
      spShaderCompilerSetup setup;
      setup.m_eStage = RHI::spShaderStage::PixelShader;
      setup.m_PredefinedMacros.PushBack({"SP_FEATURE_VERTEX_SKINNING", "SP_OFF"});

      m_pPixelShader = spShaderManager::GetSingleton()->CompileShader(m_hShader, setup);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pPixelShader->SetDebugName("MeshRenderFeature_PixelShader");
#endif
    }

    {
      m_pShaderProgram = pDevice->GetResourceFactory()->CreateShaderProgram();
      m_pShaderProgram->Attach(m_pVertexShader);
      m_pShaderProgram->Attach(m_pPixelShader);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pShaderProgram->SetDebugName("MeshRenderFeature_ShaderProgram");
#endif
    }

    {
      RHI::spResourceLayoutDescription desc{};

      RHI::spResourceLayoutElementDescription perFrameBuffer{};
      perFrameBuffer.m_sName = ezMakeHashedString("Buffer_PerFrame");
      perFrameBuffer.m_eType = RHI::spShaderResourceType::ConstantBuffer;
      perFrameBuffer.m_eShaderStage = RHI::spShaderStage::VertexShader | RHI::spShaderStage::PixelShader;
      perFrameBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      RHI::spResourceLayoutElementDescription perViewBuffer{};
      perViewBuffer.m_sName = ezMakeHashedString("Buffer_PerView");
      perViewBuffer.m_eType = RHI::spShaderResourceType::ConstantBuffer;
      perViewBuffer.m_eShaderStage = RHI::spShaderStage::VertexShader | RHI::spShaderStage::PixelShader;
      perViewBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      RHI::spResourceLayoutElementDescription perInstanceBuffer{};
      perInstanceBuffer.m_sName = ezMakeHashedString("Buffer_PerInstance");
      perInstanceBuffer.m_eType = RHI::spShaderResourceType::ReadOnlyStructuredBuffer;
      perInstanceBuffer.m_eShaderStage = RHI::spShaderStage::VertexShader;
      perInstanceBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      desc.m_Elements.PushBack(perFrameBuffer);
      desc.m_Elements.PushBack(perViewBuffer);
      desc.m_Elements.PushBack(perInstanceBuffer);

      m_pResourceLayout = pDevice->GetResourceFactory()->CreateResourceLayout(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      m_pResourceLayout->SetDebugName("MeshRenderFeature_ResourceLayout");
#endif
    }
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshRenderFeature);
