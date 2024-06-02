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

    const spRenderView* pRenderView = pRenderingContext->GetExtractionData().m_pRenderView;
    const spRenderStage* pRenderStage = pRenderingContext->GetExtractionData().m_pRenderStage;

    // if (m_PushConstantBuffer == nullptr)
    // {
    //   RHI::spBufferDescription desc;
    //   desc.m_eUsage = RHI::spBufferUsage::ConstantBuffer | RHI::spBufferUsage::Dynamic;
    //   desc.m_uiSize = sizeof(PushConstantTest);
    //
    //   m_PushConstantBuffer = cl->GetDevice()->GetResourceFactory()->CreateBuffer(desc);
    //   m_PushConstantBuffer->SetDebugName("Buffer_PerPass");
    // }

    if (m_pVertexShader == nullptr)
    {
      spShaderCompilerSetup setup;
      setup.m_eStage = RHI::spShaderStage::VertexShader;

      m_pVertexShader = spShaderManager::GetSingleton()->CompileShader(m_hShader, setup);
      m_pVertexShader->SetDebugName("vs");
    }

    if (m_pPixelShader == nullptr)
    {
      spShaderCompilerSetup setup;
      setup.m_eStage = RHI::spShaderStage::PixelShader;

      m_pPixelShader = spShaderManager::GetSingleton()->CompileShader(m_hShader, setup);
      m_pPixelShader->SetDebugName("ps");
    }

    if (m_pShaderProgram == nullptr)
    {
      m_pShaderProgram = cl->GetDevice()->GetResourceFactory()->CreateShaderProgram();
      m_pShaderProgram->Attach(m_pVertexShader);
      m_pShaderProgram->Attach(m_pPixelShader);
      m_pShaderProgram->SetDebugName("spo");
    }

    if (m_pResourceLayout == nullptr)
    {
      RHI::spResourceLayoutDescription desc{};

      RHI::spResourceLayoutElementDescription perViewBuffer{};
      perViewBuffer.m_sName = ezMakeHashedString("Buffer_PerView");
      perViewBuffer.m_eType = RHI::spShaderResourceType::ConstantBuffer;
      perViewBuffer.m_eShaderStage = RHI::spShaderStage::VertexShader;
      perViewBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::DynamicBinding;

      // RHI::spResourceLayoutElementDescription perPassBuffer{};
      // perPassBuffer.m_sName = ezMakeHashedString("Buffer_PerPass");
      // perPassBuffer.m_eType = RHI::spShaderResourceType::ConstantBuffer;
      // perPassBuffer.m_eShaderStage = RHI::spShaderStage::VertexShader;
      // perPassBuffer.m_eOptions = RHI::spResourceLayoutElementOptions::None;

      desc.m_Elements.PushBack(perViewBuffer);
      // desc.m_Elements.PushBack(perPassBuffer);

      m_pResourceLayout = cl->GetDevice()->GetResourceFactory()->CreateResourceLayout(desc);
      m_pResourceLayout->SetDebugName("layout");
    }

    if (m_pResourceSet == nullptr)
    {
      RHI::spResourceSetDescription desc{};
      desc.m_hResourceLayout = m_pResourceLayout->GetHandle();
      desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerView"), pRenderView->GetDataBuffer().GetBuffer()->GetHandle());
      // desc.m_BoundResources.Insert(ezMakeHashedString("Buffer_PerPass"), m_PushConstantBuffer->GetHandle());

      m_pResourceSet = cl->GetDevice()->GetResourceFactory()->CreateResourceSet(desc);
      m_pResourceSet->SetDebugName("set");
    }

    if (m_pGraphicPipeline == nullptr)
    {
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

      const ezUInt32 offsets[] = {pRenderView->GetDataBuffer().GetBuffer()->GetCurrentRange()->GetOffset()};
      cl->SetGraphicResourceSet(0, m_pResourceSet, 1, offsets);

      cl->SetVertexBuffer(0, mesh.GetRHIVertexBuffer());
      cl->SetIndexBuffer(mesh.GetRHIIndexBuffer(), RHI::spIndexFormat::UInt16);

      m_PushConstants.transform = pMeshRenderObject->m_Transform.GetAsMat4().GetTranspose();
      m_PushConstants.values = pMeshRenderObject->m_PreviousTransform.GetAsMat4().GetTranspose();
      cl->PushConstants(0, RHI::spShaderStage::VertexShader, &m_PushConstants, 0, sizeof(PushConstantTest));
      // cl->UpdateBuffer(m_PushConstantBuffer, 0, constants.Borrow(), 1);

      cl->DrawIndexedIndirect(mesh.GetRHIIndirectBuffer(), 0, drawCommands.GetCount(), cl->GetDevice()->GetIndexedIndirectCommandSize());
    }
    cl->PopDebugGroup();
  }

  spMeshRenderFeature::spMeshRenderFeature()
    : spRenderFeature(EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spMeshRenderFeatureExtractor))
  {
    m_hShader = ezResourceManager::LoadResource<RAI::spShaderResource>(":shaders/point.slang");
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshRenderFeature);
