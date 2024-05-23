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
    const auto pMeshRenderObject = static_cast<spMeshRenderObject*>(pRenderObject);

    const ezResourceLock resource(pMeshRenderObject->m_hMeshResource, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (!resource.IsValid())
      return; // ezLog::Error("Unable to get the mesh resource!");

    auto& mesh = resource.GetPointerNonConst()->GetLOD(0);

    mesh.CreateRHIVertexBuffer();
    mesh.CreateRHIIndexBuffer();
    mesh.CreateRHIIndirectBuffer();
    mesh.CreateRHIInputLayout();

    {
      spShaderCompilerSetup setup;
      setup.m_eStage = RHI::spShaderStage::VertexShader;

      m_pVertexShader = spShaderManager::GetSingleton()->CompileShader(m_hShader, setup);
      m_pVertexShader->SetDebugName("vs");
    }

    {
      spShaderCompilerSetup setup;
      setup.m_eStage = RHI::spShaderStage::PixelShader;

      m_pPixelShader = spShaderManager::GetSingleton()->CompileShader(m_hShader, setup);
      m_pPixelShader->SetDebugName("ps");
    }

    {
      m_pShaderProgram = cl->GetDevice()->GetResourceFactory()->CreateShaderProgram();
      m_pShaderProgram->Attach(m_pVertexShader);
      m_pShaderProgram->Attach(m_pPixelShader);
      m_pShaderProgram->SetDebugName("spo");
    }

    {
      RHI::spResourceLayoutDescription resourceLayoutDescription{};

      m_pResourceLayout = cl->GetDevice()->GetResourceFactory()->CreateResourceLayout(resourceLayoutDescription);
      m_pResourceLayout->SetDebugName("layout");
    }

    {
      RHI::spResourceSetDescription setDesc{};
      setDesc.m_hResourceLayout = m_pResourceLayout->GetHandle();

      m_pResourceSet = cl->GetDevice()->GetResourceFactory()->CreateResourceSet(setDesc);
      m_pResourceSet->SetDebugName("set");
    }

    if (m_pGraphicPipeline == nullptr)
    {
      const spRenderView* pRenderView = pRenderingContext->GetExtractionData().m_pRenderView;
      const spRenderStage* pRenderStage = pRenderingContext->GetExtractionData().m_pRenderStage;

      RHI::spGraphicPipelineDescription desc{};
      desc.m_Output = pRenderStage->GetOutputDescription(pRenderView);
      desc.m_ePrimitiveTopology = RHI::spPrimitiveTopology::Triangles;
      desc.m_RenderingState = pRenderStage->GetRenderingState(pRenderObject);
      desc.m_ShaderPipeline.m_hShaderProgram = m_pShaderProgram->GetHandle();
      desc.m_ShaderPipeline.m_InputLayouts.PushBack(mesh.GetRHIInputLayout()->GetHandle());
      desc.m_ResourceLayouts.PushBack(m_pResourceLayout->GetHandle());

      m_pGraphicPipeline = cl->GetDevice()->GetResourceFactory()->CreateGraphicPipeline(desc);
      m_pGraphicPipeline->SetDebugName("gpo");
    }

    ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper> drawCommands;
    mesh.GetDrawCommands(drawCommands);

    cl->PushDebugGroup("spMeshRenderFeature");
    {
      cl->SetGraphicPipeline(m_pGraphicPipeline);

      cl->SetGraphicResourceSet(0, m_pResourceSet);

      cl->SetVertexBuffer(0, mesh.GetRHIVertexBuffer());
      cl->SetIndexBuffer(mesh.GetRHIIndexBuffer(), RHI::spIndexFormat::UInt16);

      cl->DrawIndexedIndirect(mesh.GetRHIIndirectBuffer(), 0, drawCommands.GetCount(), cl->GetDevice()->GetIndexedIndirectCommandSize());
    }
    cl->PopDebugGroup();
  }

  spMeshRenderFeature::spMeshRenderFeature()
    : spRenderFeature(EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spMeshRenderFeatureExtractor))
  {
    m_hShader = ezResourceManager::LoadResource<RAI::spShaderResource>(":project/Shaders/sample.slang");
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshRenderFeature);
