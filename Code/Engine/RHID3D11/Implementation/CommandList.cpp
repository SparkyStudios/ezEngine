#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/CommandList.h>

#include <RHID3D11/Buffer.h>
#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Framebuffer.h>
#include <RHID3D11/Pipeline.h>
#include <RHID3D11/ResourceLayout.h>
#include <RHID3D11/ResourceSet.h>
#include <RHID3D11/Sampler.h>
#include <RHID3D11/Shader.h>
#include <RHID3D11/Swapchain.h>
#include <RHID3D11/Texture.h>

EZ_DEFINE_AS_POD_TYPE(D3D11_VIEWPORT);
EZ_DEFINE_AS_POD_TYPE(D3D11_RECT);

spCommandListD3D11::spCommandListD3D11(spDeviceD3D11* pDevice, const spCommandListDescription& description)
  : spCommandList(description)
{
  m_pDevice = pDevice;
  m_pImmediateContext = pDevice->GetD3D11DeviceContext();

  if (FAILED(m_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_pImmediateContext1))))
    ezLog::Warning("Could not get ID3D11DeviceContext1 interface, some features may not be available.");

  if (FAILED(m_pImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&m_pUserDefinedAnnotation))))
    ezLog::Warning("Could not get ID3DUserDefinedAnnotation interface, some features may not be available.");

  m_bReleased = false;
}

void spCommandListD3D11::ReleaseResource()
{
  if (m_bReleased)
    return;

  SP_RHI_DX11_RELEASE(m_pUserDefinedAnnotation);
  SP_RHI_DX11_RELEASE(m_pCommandList);
  SP_RHI_DX11_RELEASE(m_pImmediateContext1);
  SP_RHI_DX11_RELEASE(m_pImmediateContext);

  for (auto& set : m_GraphicResourceSets)
    set.m_Offsets.Clear();

  for (auto& set : m_ComputeResourceSets)
    set.m_Offsets.Clear();

  for (auto it = m_FreeBuffers.GetIterator(); it.IsValid(); it.Next())
    m_pDevice->GetResourceManager()->ReleaseResource((*it)->GetHandle());

  m_FreeBuffers.Clear();

  m_bReleased = true;
}

bool spCommandListD3D11::IsReleased() const
{
  return m_bReleased;
}

void spCommandListD3D11::Begin()
{
  SP_RHI_DX11_RELEASE(m_pCommandList);

  ClearState();
  m_bHasStarted = true;
}

void spCommandListD3D11::Dispatch(ezUInt32 uiGroupCountX, ezUInt32 uiGroupCountY, ezUInt32 uiGroupCountZ)
{
  PreDispatch();
  m_pImmediateContext->Dispatch(uiGroupCountX, uiGroupCountY, uiGroupCountZ);
}

void spCommandListD3D11::SetComputePipeline(spResourceHandle hComputePipeline)
{
  auto* pComputePipeline = m_pDevice->GetResourceManager()->GetResource<spComputePipelineD3D11>(hComputePipeline);
  EZ_ASSERT_DEV(pComputePipeline != nullptr, "Invalid compute pipeline handle");

  ClearSets(m_ComputeResourceSets);
  m_InvalidatedComputeResourceSets.Clear();

  m_pComputeShader = pComputePipeline->GetComputeShader();
  m_pImmediateContext->CSSetShader(m_pComputeShader, nullptr, 0);

  m_ComputeResourceSets.EnsureCount(pComputePipeline->GetResourceLayouts().GetCount());
  m_InvalidatedComputeResourceSets.EnsureCount(pComputePipeline->GetResourceLayouts().GetCount());

  m_pComputePipeline = pComputePipeline;
}

void spCommandListD3D11::SetGraphicPipeline(spResourceHandle hGraphicPipeline)
{
  auto* pGraphicPipeline = m_pDevice->GetResourceManager()->GetResource<spGraphicPipelineD3D11>(hGraphicPipeline);
  EZ_ASSERT_DEV(pGraphicPipeline != nullptr, "Invalid graphic pipeline handle");

  ClearSets(m_GraphicResourceSets);
  m_InvalidatedGraphicResourceSets.Clear();

  ID3D11BlendState* pBlendState = pGraphicPipeline->GetBlendState();
  ezColor blendFactor = pGraphicPipeline->GetBlendFactor();
  if (m_pBlendState != pBlendState || m_BlendFactor != blendFactor)
  {
    FLOAT blendFactors[4] = {blendFactor.r, blendFactor.g, blendFactor.b, blendFactor.a};

    m_pBlendState = pBlendState;
    m_BlendFactor = blendFactor;
    m_pImmediateContext->OMSetBlendState(m_pBlendState, blendFactors, 0xFFFFFFFFu);
  }

  ID3D11DepthStencilState* pDepthStencilState = pGraphicPipeline->GetDepthStencilState();
  ezUInt32 uiStencilRef = pGraphicPipeline->GetStencilRef();
  if (m_pDepthStencilState != pDepthStencilState || m_uiStencilRef != uiStencilRef)
  {
    m_pDepthStencilState = pDepthStencilState;
    m_uiStencilRef = uiStencilRef;
    m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, uiStencilRef);
  }

  ID3D11RasterizerState* pRasterizerState = pGraphicPipeline->GetRasterizerState();
  if (m_pRasterizerState != pRasterizerState)
  {
    m_pRasterizerState = pRasterizerState;
    m_pImmediateContext->RSSetState(m_pRasterizerState);
  }

  D3D11_PRIMITIVE_TOPOLOGY ePrimitiveTopology = pGraphicPipeline->GetPrimitiveTopology();
  if (m_ePrimitiveTopology != ePrimitiveTopology)
  {
    m_ePrimitiveTopology = ePrimitiveTopology;
    m_pImmediateContext->IASetPrimitiveTopology(m_ePrimitiveTopology);
  }

  ID3D11VertexShader* pVertexShader = pGraphicPipeline->GetVertexShader();
  if (m_pVertexShader != pVertexShader)
  {
    m_pVertexShader = pVertexShader;
    m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
  }

  ID3D11GeometryShader* pGeometryShader = pGraphicPipeline->GetGeometryShader();
  if (m_pGeometryShader != pGeometryShader)
  {
    m_pGeometryShader = pGeometryShader;
    m_pImmediateContext->GSSetShader(m_pGeometryShader, nullptr, 0);
  }

  ID3D11HullShader* pHullShader = pGraphicPipeline->GetHullShader();
  if (m_pHullShader != pHullShader)
  {
    m_pHullShader = pHullShader;
    m_pImmediateContext->HSSetShader(m_pHullShader, nullptr, 0);
  }

  ID3D11DomainShader* pDomainShader = pGraphicPipeline->GetDomainShader();
  if (m_pDomainShader != pDomainShader)
  {
    m_pDomainShader = pDomainShader;
    m_pImmediateContext->DSSetShader(m_pDomainShader, nullptr, 0);
  }

  ID3D11PixelShader* pPixelShader = pGraphicPipeline->GetPixelShader();
  if (m_pPixelShader != pPixelShader)
  {
    m_pPixelShader = pPixelShader;
    m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
  }

  m_VertexStrides = pGraphicPipeline->GetVertexStrides();
  if (!m_VertexStrides.IsEmpty())
  {
    ezUInt32 uiVertexStridesCount = m_VertexStrides.GetCount();
    m_VertexBuffers.EnsureCount(uiVertexStridesCount);
    m_VertexOffsets.EnsureCount(uiVertexStridesCount);
  }

  m_GraphicResourceSets.EnsureCount(pGraphicPipeline->GetResourceLayouts().GetCount());
  m_InvalidatedGraphicResourceSets.EnsureCount(pGraphicPipeline->GetResourceLayouts().GetCount());

  m_pGraphicPipeline = pGraphicPipeline;
}

void spCommandListD3D11::SetScissorRect(ezUInt32 uiSlot, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  m_bScissorRectsChanged = true;
  m_ScissorRects.EnsureCount(uiSlot + 1);

  m_ScissorRects[uiSlot].left = uiX;
  m_ScissorRects[uiSlot].right = uiX + uiWidth;
  m_ScissorRects[uiSlot].top = uiY;
  m_ScissorRects[uiSlot].bottom = uiY + uiHeight;
}

void spCommandListD3D11::SetViewport(ezUInt32 uiSlot, const spViewport& viewport)
{
  m_bViewportsChanged = true;
  m_Viewports.EnsureCount(uiSlot + 1);

  m_Viewports[uiSlot].TopLeftX = viewport.m_iX;
  m_Viewports[uiSlot].TopLeftY = viewport.m_iY;
  m_Viewports[uiSlot].Width = viewport.m_uiWidth;
  m_Viewports[uiSlot].Height = viewport.m_uiHeight;
  m_Viewports[uiSlot].MinDepth = viewport.m_fMinDepth;
  m_Viewports[uiSlot].MaxDepth = viewport.m_fMaxDepth;
}

void spCommandListD3D11::PushDebugGroup(const ezString& sName)
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (m_pUserDefinedAnnotation == nullptr)
    return;

  ezStringWChar wsMarker(sName);
  m_pUserDefinedAnnotation->BeginEvent(wsMarker.GetData());
#endif
}

void spCommandListD3D11::PopDebugGroup()
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (m_pUserDefinedAnnotation == nullptr)
    return;

  m_pUserDefinedAnnotation->EndEvent();
#endif
}

void spCommandListD3D11::InsertDebugMarker(const ezString& sName)
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (m_pUserDefinedAnnotation == nullptr)
    return;

  ezStringWChar wsMarker(sName);
  m_pUserDefinedAnnotation->SetMarker(wsMarker.GetData());
#endif
}

void spCommandListD3D11::End()
{
  EZ_ASSERT_DEV(m_pCommandList == nullptr, "Invalid call to End");

  m_pImmediateContext->FinishCommandList(false, &m_pCommandList);
  m_pCommandList->SetPrivateData(WKPDID_D3DDebugObjectName, m_sDebugName.GetElementCount(), m_sDebugName.GetData());

  ResetManagedState();
  m_bHasStarted = false;
}

void spCommandListD3D11::ClearColorTargetInternal(ezUInt32 uiIndex, ezColor clearColor)
{
  float Color[4] = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};
  m_pImmediateContext->ClearRenderTargetView(m_pFramebuffer->GetRenderTargetView(uiIndex), Color);
}

void spCommandListD3D11::ClearDepthStencilTargetInternal(float fClearDepth, ezUInt8 uiClearStencil)
{
  m_pImmediateContext->ClearDepthStencilView(m_pFramebuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, fClearDepth, uiClearStencil);
}

void spCommandListD3D11::DrawInternal(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart)
{
  PreDraw();

  if (uiInstanceCount == 1 && uiInstanceStart == 0)
    m_pImmediateContext->Draw(uiVertexCount, uiVertexStart);
  else
    m_pImmediateContext->DrawInstanced(uiVertexCount, uiInstanceCount, uiVertexStart, uiInstanceStart);
}

void spCommandListD3D11::DrawIndexedInternal(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart)
{
  PreDraw();

  EZ_ASSERT_DEV(m_pIndexBuffer != nullptr, "Index buffer is not set.");

  if (uiInstanceCount == 1 && uiInstanceStart == 0)
    m_pImmediateContext->DrawIndexed(uiIndexCount, uiIndexStart, uiVertexOffset);
  else
    m_pImmediateContext->DrawIndexedInstanced(uiIndexCount, uiIndexStart, uiVertexOffset, uiInstanceCount, uiInstanceStart);
}

void spCommandListD3D11::DrawIndirectInternal(spBuffer* pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
{
  PreDraw();

  auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pIndirectBuffer);
  ezUInt32 uiCurrentOffset = uiOffset;

  for (ezUInt32 i = 0; i < uiDrawCount; i++)
  {
    m_pImmediateContext->DrawInstancedIndirect(pBufferD3D11->GetD3D11Buffer(), uiCurrentOffset);
    uiCurrentOffset += uiStride;
  }
}

void spCommandListD3D11::DrawIndexedIndirectInternal(spBuffer* pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
{
  PreDraw();

  auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pIndirectBuffer);
  ezUInt32 uiCurrentOffset = uiOffset;

  for (ezUInt32 i = 0; i < uiDrawCount; i++)
  {
    m_pImmediateContext->DrawIndexedInstancedIndirect(pBufferD3D11->GetD3D11Buffer(), uiCurrentOffset);
    uiCurrentOffset += uiStride;
  }
}

void spCommandListD3D11::DispatchIndirectInternal(spBuffer* pIndirectBuffer, ezUInt32 uiOffset)
{
  PreDispatch();

  auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pIndirectBuffer);
  m_pImmediateContext->DispatchIndirect(pBufferD3D11->GetD3D11Buffer(), uiOffset);
}

void spCommandListD3D11::ResolveTextureInternal(spTexture* pSource, spTexture* pDestination)
{
  auto* pSourceD3D11 = ezStaticCast<spTextureD3D11*>(pSource);
  auto* pDestinationD3D11 = ezStaticCast<spTextureD3D11*>(pDestination);

  m_pImmediateContext->ResolveSubresource(pSourceD3D11->GetD3D11Texture(), 0, pDestinationD3D11->GetD3D11Texture(), 0, pDestinationD3D11->GetDXGIFormat());
}

void spCommandListD3D11::SetFramebufferInternal(spFramebuffer* pFramebuffer)
{
  auto* pFramebufferD3D11 = ezStaticCast<spFramebufferD3D11*>(pFramebuffer);

  // TODO: Swapchain references

  for (ezUInt32 i = 0, l = pFramebufferD3D11->GetColorTargets().GetCount(); i < l; ++i)
  {
    UnbindSRVTexture(spTextureViewDescription(pFramebufferD3D11->GetColorTarget(i)));
  }

  const auto& views = pFramebufferD3D11->GetRenderTargetViews();
  m_pImmediateContext->OMSetRenderTargets(views.GetCount(), views.GetPtr(), pFramebufferD3D11->GetDepthStencilView());
}

void spCommandListD3D11::SetIndexBufferInternal(spBuffer* pIndexBuffer, ezEnum<spIndexFormat> eFormat, ezUInt32 uiOffset)
{
  auto* pIndexBufferD3D11 = ezStaticCast<spBufferD3D11*>(pIndexBuffer);

  if (m_pIndexBuffer != pIndexBuffer || m_uiIndexBufferOffset != uiOffset)
  {
    m_pIndexBuffer = pIndexBufferD3D11;
    m_uiIndexBufferOffset = uiOffset;

    UnbindUAVBuffer(pIndexBufferD3D11);
    m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer->GetD3D11Buffer(), spToD3D11(eFormat), uiOffset);
  }
}

void spCommandListD3D11::SetComputeResourceSetInternal(ezUInt32 uiSlot, spResourceSet* pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
{
  spCommandListResourceSet& set = m_ComputeResourceSets[uiSlot];

  if (set.m_hResourceSet == pResourceSet->GetHandle() && ezMemoryUtils::IsEqual(set.m_Offsets.GetPtr(), pDynamicOffsets, uiDynamicOffsetCount))
    return;

  set.m_Offsets.Clear();
  set = spCommandListResourceSet(pResourceSet->GetHandle(), uiDynamicOffsetCount, pDynamicOffsets);
  ActivateResourceSet(uiSlot, set, true);
}

void spCommandListD3D11::SetGraphicResourceSetInternal(ezUInt32 uiSlot, spResourceSet* pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
{
  spCommandListResourceSet& set = m_ComputeResourceSets[uiSlot];

  if (set.m_hResourceSet == pResourceSet->GetHandle() && ezMemoryUtils::IsEqual(set.m_Offsets.GetPtr(), pDynamicOffsets, uiDynamicOffsetCount))
    return;

  set.m_Offsets.Clear();
  set = spCommandListResourceSet(pResourceSet->GetHandle(), uiDynamicOffsetCount, pDynamicOffsets);
  ActivateResourceSet(uiSlot, set, true);
}

void spCommandListD3D11::SetVertexBufferInternal(ezUInt32 uiSlot, spBuffer* pVertexBuffer, ezUInt32 uiOffset)
{
  auto* pBuffer = ezStaticCast<spBufferD3D11*>(pVertexBuffer);

  if (m_VertexBuffers[uiSlot] != pBuffer->GetD3D11Buffer() || m_VertexOffsets[uiSlot] != uiOffset)
  {
    m_bIsVertexBindingsDirty = true;
    UnbindUAVBuffer(pBuffer);

    m_VertexBuffers[uiSlot] = pBuffer->GetD3D11Buffer();
    m_VertexOffsets[uiSlot] = uiOffset;

    m_uiNumVertexBuffers = ezMath::Max(m_uiNumVertexBuffers, uiSlot + 1);
  }
}

void spCommandListD3D11::UpdateBufferInternal(spBuffer* pBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize)
{
  auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pBuffer);

  bool bDynamic = pBuffer->GetUsage().IsSet(spBufferUsage::Dynamic);
  bool bStaging = pBuffer->GetUsage().IsSet(spBufferUsage::Staging);
  bool bConstantBuffer = pBuffer->GetUsage().IsSet(spBufferUsage::ConstantBuffer);
  bool bUseMap = bDynamic;
  bool bUpdateFullBuffer = uiOffset == 0 && uiSize == pBufferD3D11->GetSize();
  bool bUseUpdateSubresource = !bDynamic && !bStaging && (!bConstantBuffer || bUpdateFullBuffer);

  if (bUseUpdateSubresource)
  {
    D3D11_BOX region = {uiOffset, 0, 0, uiOffset + uiSize, 1, 1};
    bool bHasRegion = !bConstantBuffer;

    if (uiOffset == 0)
    {
      m_pImmediateContext->UpdateSubresource(pBufferD3D11->GetD3D11Buffer(), 0, bHasRegion ? &region : nullptr, pSourceData, 0, 0);
    }
    else
    {
      bool bNeedWorkAround = !m_pDevice->GetCapabilities().m_bSupportCommandLists;
      const void* pAdjustedData = pSourceData;

      if (bNeedWorkAround)
      {
        EZ_ASSERT_DEV(region.top == 0 && region.front == 0, "Invalid region.");
        pAdjustedData = reinterpret_cast<const ezUInt8*>(pSourceData) - region.left;
      }

      m_pImmediateContext->UpdateSubresource(pBufferD3D11->GetD3D11Buffer(), 0, &region, pAdjustedData, 0, 0);
    }
  }
  else if (bUseMap && bUpdateFullBuffer) // Can only update full buffer with WriteDiscard
  {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_pImmediateContext->Map(pBufferD3D11->GetD3D11Buffer(), 0, spToD3D11(spMapAccess::Write, bDynamic), 0, &mappedResource);

    ezMemoryUtils::RawByteCopy(mappedResource.pData, pSourceData, uiSize);

    m_pImmediateContext->Unmap(pBufferD3D11->GetD3D11Buffer(), 0);
  }
  else
  {
    spBufferD3D11* pStagingBufferD3D11 = GetFreeStagingBuffer(uiSize);
    m_pDevice->UpdateBuffer(pStagingBufferD3D11->GetHandle(), 0, pSourceData, uiSize);
    CopyBuffer(pStagingBufferD3D11->GetHandle(), 0, pBuffer->GetHandle(), uiOffset, uiSize);
    m_SubmittedStagingBuffers.PushBack(pStagingBufferD3D11);
  }
}

void spCommandListD3D11::CopyBufferInternal(spBuffer* pSourceBuffer, ezUInt32 uiSourceOffset, spBuffer* pDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize)
{
  auto* pDestBufferD3D11 = ezStaticCast<spBufferD3D11*>(pDestBuffer);
  auto* pSourceBufferD3D11 = ezStaticCast<spBufferD3D11*>(pSourceBuffer);

  D3D11_BOX srcBox = {uiSourceOffset, 0, 0, uiSourceOffset + uiSize, 1, 1};
  m_pImmediateContext->CopySubresourceRegion(pDestBufferD3D11->GetD3D11Buffer(), 0, uiDestOffset, 0, 0, pSourceBufferD3D11->GetD3D11Buffer(), uiSourceOffset, &srcBox);
}

void spCommandListD3D11::CopyTextureInternal(spTexture* pSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, spTexture* pDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount)
{
  auto* pSourceTextureD3D11 = ezStaticCast<spTextureD3D11*>(pSourceTexture);
  auto* pDestinationTextureD3D11 = ezStaticCast<spTextureD3D11*>(pDestinationTexture);

  ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(pSourceTexture->GetFormat()) ? 4u : 1u;
  ezUInt32 uiClampedWidth = ezMath::Max(uiWidth, uiBlockSize);
  ezUInt32 uiClampedHeight = ezMath::Max(uiHeight, uiBlockSize);

  D3D11_BOX region;
  bool bHasRegion = false;
  if (uiSourceX != 0 || uiSourceY != 0 || uiSourceZ != 0 || uiClampedWidth != pSourceTextureD3D11->GetWidth() || uiClampedHeight != pSourceTextureD3D11->GetHeight() || uiDepth != pSourceTextureD3D11->GetDepth())
  {
    bHasRegion = true;
    region = {uiSourceX, uiSourceY, uiSourceZ, uiClampedWidth + uiSourceX, uiClampedHeight + uiSourceY, uiDepth + uiSourceZ};
  }

  for (ezUInt32 i = 0; i < uiLayerCount; ++i)
  {
    ezUInt32 uiSrcSubresource = spTextureHelper::CalculateSubresource(pSourceTexture, uiSourceMipLevel, uiSourceBaseArrayLayer + i);
    ezUInt32 uiDestSubresource = spTextureHelper::CalculateSubresource(pDestinationTexture, uiDestMipLevel, uiDestBaseArrayLayer + i);

    m_pImmediateContext->CopySubresourceRegion(pDestinationTextureD3D11->GetD3D11Texture(), uiDestSubresource, uiDestX, uiDestY, uiDestZ, pSourceTextureD3D11->GetD3D11Texture(), uiSrcSubresource, bHasRegion ? &region : nullptr);
  }
}

void spCommandListD3D11::GenerateMipmapsInternal(spTexture* pTexture)
{
  auto* pTextureD3D11 = ezStaticCast<spTextureD3D11*>(pTexture);

  spResourceHandle hTextureView = m_pDevice->GetTextureSamplerManager()->GetFullTextureView(pTexture->GetHandle());
  auto* pTextureViewD3D11 = m_pDevice->GetResourceManager()->GetResource<spTextureViewD3D11*>(hTextureView);

  ID3D11ShaderResourceView* pSRV = pTextureViewD3D11->GetShaderResourceView();
  m_pImmediateContext->GenerateMips(pSRV);
}

void spCommandListD3D11::Reset()
{
  if (m_bHasStarted)
  {
    m_pImmediateContext->ClearState();
    m_pImmediateContext->FinishCommandList(false, &m_pCommandList);
  }

  SP_RHI_DX11_RELEASE(m_pCommandList);

  ResetManagedState();
  m_bHasStarted = false;
}

void spCommandListD3D11::ClearState()
{
  ClearCachedState();
  m_pImmediateContext->ClearState();
  ResetManagedState();
}

void spCommandListD3D11::ResetManagedState()
{
  m_uiNumVertexBuffers = 0;
  m_VertexBuffers.Clear();
  m_VertexStrides.Clear();
  m_VertexOffsets.Clear();

  m_pFramebuffer = nullptr;

  m_bViewportsChanged = false;
  m_Viewports.Clear();
  m_bScissorRectsChanged = false;
  m_ScissorRects.Clear();

  m_pIndexBuffer = nullptr;
  m_pGraphicPipeline = nullptr;
  m_pBlendState = nullptr;
  m_pDepthStencilState = nullptr;
  m_pRasterizerState = nullptr;
  m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
  m_pInputLayout = nullptr;
  m_pVertexShader = nullptr;
  m_pGeometryShader = nullptr;
  m_pHullShader = nullptr;
  m_pDomainShader = nullptr;
  m_pPixelShader = nullptr;
  m_pComputeShader = nullptr;

  ClearSets(m_GraphicResourceSets);

  m_CachedVertexConstantBuffers.Clear();
  m_CachedVertexTextureViews.Clear();
  m_CachedVertexSamplerStates.Clear();

  m_CachedPixelConstantBuffers.Clear();
  m_CachedPixelTextureViews.Clear();
  m_CachedPixelSamplerStates.Clear();

  m_pComputePipeline = nullptr;
  ClearSets(m_ComputeResourceSets);

  m_BoundSRVs.Clear();
  m_BoundUAVs.Clear();
}

void spCommandListD3D11::ClearSets(ezDynamicArray<spCommandListResourceSet>& sets)
{
  for (ezUInt32 i = 0, l = sets.GetCount(); i < l; i++)
    sets[i].m_Offsets.Clear();

  sets.Clear();
}

void spCommandListD3D11::ActivateResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet, bool bCompute)
{
  spResourceSetD3D11* pResourceSet = m_pDevice->GetResourceManager()->GetResource<spResourceSetD3D11>(resourceSet.m_hResourceSet);
  EZ_ASSERT_DEV(pResourceSet != nullptr, "Invalid resource set handle");

  ezUInt32 uiConstantBufferBase = GetConstantBufferBase(uiSlot, bCompute);
  ezUInt32 uiUnorderedAccessViewBase = GetUnorderedAccessViewBase(uiSlot, bCompute);
  ezUInt32 uiTextureBase = GetTextureBase(uiSlot, bCompute);
  ezUInt32 uiSamplerBase = GetSamplerBase(uiSlot, bCompute);

  spResourceLayoutD3D11* pLayout = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutD3D11>(pResourceSet->GetLayout());
  EZ_ASSERT_DEV(pLayout != nullptr, "Invalid resource layout handle");

  ezDynamicArray<spShaderResource*> resources = pResourceSet->GetResources();
  ezUInt32 uiDynamicOffsetIndex = 0;

  for (ezUInt32 i = 0, l = resources.GetCount(); i < l; ++i)
  {
    spShaderResource* pResource = resources[i];
    ezUInt32 uiBufferOffset = 0;

    if (pLayout->IsDynamicBuffer(i))
    {
      uiBufferOffset = resourceSet.m_Offsets[uiDynamicOffsetIndex];
      uiDynamicOffsetIndex++;
    }

    spResourceLayoutD3D11::BindingInfo layoutBindingInfo = pLayout->GetBinding(i);
    switch (layoutBindingInfo.m_eResourceType)
    {
      case spShaderResourceType::ConstantBuffer:
      {
        spBufferRangeD3D11* pBufferRange = GetBufferRange(pResource, uiBufferOffset);
        BindConstantBuffer(pBufferRange, uiConstantBufferBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage);
        break;
      }
      case spShaderResourceType::ReadOnlyStructuredBuffer:
      {
        spBufferRangeD3D11* pBufferRange = GetBufferRange(pResource, uiBufferOffset);
        BindStorageBufferView(pBufferRange, uiTextureBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage);
        break;
      }
      case spShaderResourceType::ReadWriteStructuredBuffer:
      {
        spBufferRangeD3D11* pBufferRange = GetBufferRange(pResource, uiBufferOffset);
        ID3D11UnorderedAccessView* pUAV = pBufferRange->GetBuffer()->GetUnorderedAccessView(pBufferRange->GetOffset(), pBufferRange->GetSize());
        BindUnorderedAccessView(nullptr, pBufferRange->GetBuffer(), pUAV, uiUnorderedAccessViewBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage, uiSlot);
        break;
      }
      case spShaderResourceType::ReadOnlyTexture:
      {
        spTextureViewD3D11* pTextureView = ezStaticCast<spTextureViewD3D11*>(spTextureSamplerManager::GetTextureView(m_pDevice, pResource));
        spTextureD3D11* pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(pResourceSet->GetLayout());
        UnbindUAVTexture(pTextureView->GetDescription());
        BindTextureView(pTextureView, uiTextureBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage, uiSlot);
        break;
      }
      case spShaderResourceType::ReadWriteTexture:
      {
        spTextureViewD3D11* pTextureView = ezStaticCast<spTextureViewD3D11*>(spTextureSamplerManager::GetTextureView(m_pDevice, pResource));
        spTextureD3D11* pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(pResourceSet->GetLayout());
        UnbindSRVTexture(pTextureView->GetDescription());
        BindUnorderedAccessView(pTexture, nullptr, pTextureView->GetUnorderedAccessView(), uiUnorderedAccessViewBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage, uiSlot);
        break;
      }
      case spShaderResourceType::Sampler:
      {
        spSamplerD3D11* pSampler = m_pDevice->GetResourceManager()->GetResource<spSamplerD3D11>(pResourceSet->GetLayout());
        BindSampler(pSampler, uiSamplerBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }
}

spBufferD3D11* spCommandListD3D11::GetFreeStagingBuffer(ezUInt32 uiSize)
{
  for (auto it = m_FreeBuffers.GetIterator(); it.IsValid(); it.Next())
  {
    if ((*it)->GetSize() >= uiSize)
    {
      m_FreeBuffers.Remove(it);
      return (*it);
    }
  }

  spResourceHandle hBuffer = m_pDevice->GetResourceFactory()->CreateBuffer(spBufferDescription(uiSize, spBufferUsage::Staging));
  spBufferD3D11* pBuffer = m_pDevice->GetResourceManager()->GetResource<spBufferD3D11>(hBuffer);

  return pBuffer;
}

ezUInt32 spCommandListD3D11::GetConstantBufferBase(ezUInt32 uiSlot, bool bCompute)
{
  ezArrayPtr<spResourceLayoutD3D11* const> layouts = bCompute ? static_cast<spComputePipelineD3D11*>(m_pComputePipeline)->GetResourceLayouts().GetArrayPtr() : static_cast<spGraphicPipelineD3D11*>(m_pGraphicPipeline)->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i]->GetConstantBufferCount();
  }

  return uiBase;
}

ezUInt32 spCommandListD3D11::GetUnorderedAccessViewBase(ezUInt32 uiSlot, bool bCompute)
{
  ezArrayPtr<spResourceLayoutD3D11* const> layouts = bCompute ? static_cast<spComputePipelineD3D11*>(m_pComputePipeline)->GetResourceLayouts().GetArrayPtr() : static_cast<spGraphicPipelineD3D11*>(m_pGraphicPipeline)->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i]->GetStorageBufferCount();
  }

  return uiBase;
}

ezUInt32 spCommandListD3D11::GetTextureBase(ezUInt32 uiSlot, bool bCompute)
{
  ezArrayPtr<spResourceLayoutD3D11* const> layouts = bCompute ? static_cast<spComputePipelineD3D11*>(m_pComputePipeline)->GetResourceLayouts().GetArrayPtr() : static_cast<spGraphicPipelineD3D11*>(m_pGraphicPipeline)->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i]->GetTextureCount();
  }

  return uiBase;
}

ezUInt32 spCommandListD3D11::GetSamplerBase(ezUInt32 uiSlot, bool bCompute)
{
  ezArrayPtr<spResourceLayoutD3D11* const> layouts = bCompute ? static_cast<spComputePipelineD3D11*>(m_pComputePipeline)->GetResourceLayouts().GetArrayPtr() : static_cast<spGraphicPipelineD3D11*>(m_pGraphicPipeline)->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i]->GetSamplerCount();
  }

  return uiBase;
}

spBufferRangeD3D11* spCommandListD3D11::GetBufferRange(spShaderResource* pResource, ezUInt32 uiOffset)
{
  spBufferRangeD3D11* pBufferRange = nullptr;
  spBufferRangeD3D11* pBufferRangeSource = nullptr;

  if (pResource->IsInstanceOf<spBufferD3D11>())
  {
    auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pResource);
    pBufferRangeSource = m_pDevice->GetResourceManager()->GetResource<spBufferRangeD3D11*>(pBufferD3D11->GetCurrentBuffer());
  }
  else if (pResource->IsInstanceOf<spBufferRangeD3D11>())
  {
    pBufferRangeSource = ezStaticCast<spBufferRangeD3D11*>(pResource);
  }

  pBufferRange = EZ_DEFAULT_NEW(spBufferRangeD3D11, static_cast<spDeviceD3D11*>(m_pDevice), spBufferRangeDescription(pBufferRangeSource->GetBuffer()->GetHandle(), uiOffset + pBufferRangeSource->GetOffset(), pBufferRangeSource->GetSize()));
  m_pDevice->GetResourceManager()->RegisterResource(pBufferRange);

  return pBufferRange;
}

void spCommandListD3D11::BindConstantBuffer(spBufferRangeD3D11* pBufferRange, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages)
{
  if (eStages.IsSet(spShaderStage::VertexShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedConstantBuffers)
    {
      if (m_CachedVertexConstantBuffers[uiSlot] != pBufferRange)
      {
        m_CachedVertexConstantBuffers[uiSlot] = pBufferRange;
        bBind = true;
      }
    }
    else
    {
      bBind = true;
    }

    if (bBind)
    {
      if (pBufferRange->IsFullRange())
      {
        ID3D11Buffer* pBuffer = pBufferRange->GetBuffer()->GetD3D11Buffer();
        m_pImmediateContext->VSSetConstantBuffers(uiSlot, 1, &pBuffer);
      }
      else
      {
        PackRangeParams(pBufferRange);
        if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
          m_pImmediateContext->VSSetConstantBuffers(uiSlot, 1, nullptr);

        m_pImmediateContext1->VSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
      }
    }
  }

  if (eStages.IsSet(spShaderStage::GeometryShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer()->GetD3D11Buffer();
      m_pImmediateContext->GSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pImmediateContext->GSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pImmediateContext1->GSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }

  if (eStages.IsSet(spShaderStage::HullShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer()->GetD3D11Buffer();
      m_pImmediateContext->HSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pImmediateContext->HSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pImmediateContext1->HSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }

  if (eStages.IsSet(spShaderStage::DomainShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer()->GetD3D11Buffer();
      m_pImmediateContext->DSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pImmediateContext->DSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pImmediateContext1->DSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }

  if (eStages.IsSet(spShaderStage::PixelShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedConstantBuffers)
    {
      if (m_CachedPixelConstantBuffers[uiSlot] != pBufferRange)
      {
        m_CachedPixelConstantBuffers[uiSlot] = pBufferRange;
        bBind = true;
      }
    }
    else
    {
      bBind = true;
    }

    if (bBind)
    {
      if (pBufferRange->IsFullRange())
      {
        ID3D11Buffer* pBuffer = pBufferRange->GetBuffer()->GetD3D11Buffer();
        m_pImmediateContext->PSSetConstantBuffers(uiSlot, 1, &pBuffer);
      }
      else
      {
        PackRangeParams(pBufferRange);
        if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
          m_pImmediateContext->PSSetConstantBuffers(uiSlot, 1, nullptr);

        m_pImmediateContext1->PSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
      }
    }
  }

  if (eStages.IsSet(spShaderStage::ComputeShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer()->GetD3D11Buffer();
      m_pImmediateContext->CSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pImmediateContext->CSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pImmediateContext1->CSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }
}

void spCommandListD3D11::BindStorageBufferView(spBufferRangeD3D11* pBufferRange, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages)
{
  UnbindUAVBuffer(pBufferRange->GetBuffer());

  ID3D11ShaderResourceView* pSRV = pBufferRange->GetBuffer()->GetShaderResourceView(pBufferRange->GetOffset(), pBufferRange->GetSize());

  if (eStages.IsSet(spShaderStage::VertexShader))
    m_pImmediateContext->VSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::GeometryShader))
    m_pImmediateContext->GSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::HullShader))
    m_pImmediateContext->HSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::DomainShader))
    m_pImmediateContext->DSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::PixelShader))
    m_pImmediateContext->PSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::ComputeShader))
    m_pImmediateContext->CSSetShaderResources(uiSlot, 1, &pSRV);
}

void spCommandListD3D11::BindTextureView(spTextureViewD3D11* pTextureView, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages, ezUInt32 uiSetSlot)
{
  ID3D11ShaderResourceView* pSRV = pTextureView != nullptr ? pTextureView->GetShaderResourceView() : nullptr;
  if (pSRV != nullptr)
  {
    BoundTextureInfo info = {uiSlot, eStages, uiSetSlot};
    m_BoundSRVs.Insert(pTextureView->GetDescription(), info);
  }

  if (eStages.IsSet(spShaderStage::VertexShader))
  {
    bool bBind = true;
    if (uiSlot < s_uiMaxCachedTextureViews)
    {
      if (m_CachedVertexTextureViews[uiSlot] != pTextureView)
      {
        m_CachedVertexTextureViews[uiSlot] = pTextureView;
        bBind = true;
      }
    }
    else
    {
      bBind = true;
    }

    if (bBind)
    {
      m_pImmediateContext->VSSetShaderResources(uiSlot, 1, &pSRV);
    }
  }

  if (eStages.IsSet(spShaderStage::GeometryShader))
  {
    m_pImmediateContext->GSSetShaderResources(uiSlot, 1, &pSRV);
  }

  if (eStages.IsSet(spShaderStage::HullShader))
  {
    m_pImmediateContext->HSSetShaderResources(uiSlot, 1, &pSRV);
  }

  if (eStages.IsSet(spShaderStage::DomainShader))
  {
    m_pImmediateContext->DSSetShaderResources(uiSlot, 1, &pSRV);
  }

  if (eStages.IsSet(spShaderStage::PixelShader))
  {
    bool bBind = true;
    if (uiSlot < s_uiMaxCachedTextureViews)
    {
      if (m_CachedPixelTextureViews[uiSlot] != pTextureView)
      {
        m_CachedPixelTextureViews[uiSlot] = pTextureView;
        bBind = true;
      }
    }
    else
    {
      bBind = true;
    }

    if (bBind)
    {
      m_pImmediateContext->PSSetShaderResources(uiSlot, 1, &pSRV);
    }
  }

  if (eStages.IsSet(spShaderStage::ComputeShader))
  {
    m_pImmediateContext->CSSetShaderResources(uiSlot, 1, &pSRV);
  }
}

void spCommandListD3D11::BindUnorderedAccessView(spTextureD3D11* pTexture, spBufferD3D11* pBuffer, ID3D11UnorderedAccessView* pUAV, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages, ezUInt32 uiSetSlot)
{
  bool bCompute = eStages == spShaderStage::ComputeShader;
  EZ_ASSERT_DEV(bCompute || !eStages.IsSet(spShaderStage::ComputeShader), "Unordered access views cannot be bound on compute and graphic stages at the same time.");
  EZ_ASSERT_DEV(pTexture == nullptr || pBuffer == nullptr, "Cannot bind unordered access views to textures and buffers at the same time.");

  if (pTexture != nullptr && pUAV != nullptr)
  {
    BoundTextureInfo info = {uiSlot, eStages, uiSetSlot};
    spTextureViewDescription desc = spTextureViewDescription(pTexture);
    m_BoundUAVs.Insert(desc, info);
  }

  ezUInt32 uiBaseSlot = 0;
  if (!bCompute && !m_CachedPixelSamplerStates.IsEmpty())
    uiBaseSlot = m_pFramebuffer->GetColorTargets().GetCount();

  ezUInt32 uiActualSlot = uiBaseSlot + uiSlot;
  if (pBuffer != nullptr)
    TrackBoundUAVBuffer(pBuffer, uiActualSlot, bCompute);

  if (bCompute)
    m_pImmediateContext->CSSetUnorderedAccessViews(uiActualSlot, 1, &pUAV, nullptr);
  else
    m_pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, uiActualSlot, 1, &pUAV, nullptr);
}

void spCommandListD3D11::BindSampler(spSamplerD3D11* pSampler, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages)
{
  ID3D11SamplerState* pD3D11SamplerState = pSampler->GetSamplerState()->GetD3D11SamplerState();

  if (eStages.IsSet(spShaderStage::VertexShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedSamplerStates)
    {
      if (m_CachedVertexSamplerStates[uiSlot] != pSampler)
      {
        m_CachedVertexSamplerStates[uiSlot] = pSampler;
        bBind = true;
      }
    }
    else
    {
      bBind = true;
    }

    if (bBind)
    {
      m_pImmediateContext->VSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
    }
  }

  if (eStages.IsSet(spShaderStage::GeometryShader))
  {
    m_pImmediateContext->GSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
  }

  if (eStages.IsSet(spShaderStage::HullShader))
  {
    m_pImmediateContext->HSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
  }

  if (eStages.IsSet(spShaderStage::DomainShader))
  {
    m_pImmediateContext->DSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
  }

  if (eStages.IsSet(spShaderStage::PixelShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedSamplerStates)
    {
      if (m_CachedPixelSamplerStates[uiSlot] != pSampler)
      {
        m_CachedPixelSamplerStates[uiSlot] = pSampler;
        bBind = true;
      }
    }
    else
    {
      bBind = true;
    }

    if (bBind)
    {
      m_pImmediateContext->PSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
    }
  }

  if (eStages.IsSet(spShaderStage::ComputeShader))
  {
    m_pImmediateContext->CSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
  }
}

void spCommandListD3D11::UnbindSRVTexture(const spTextureViewDescription& desc)
{
  for (ezUInt32 i = 0, l = m_BoundSRVs.GetCount(); i < l; ++i)
  {
    if (m_BoundSRVs.GetKey(i) == desc)
    {
      const BoundTextureInfo& info = m_BoundSRVs.GetValue(i);
      BindTextureView(nullptr, info.m_uiSlot, info.m_eStages, 0);

      if (info.m_eStages.IsSet(spShaderStage::ComputeShader))
        m_InvalidatedComputeResourceSets[info.m_uiResourceSet] = true;
      else
        m_InvalidatedGraphicResourceSets[info.m_uiResourceSet] = true;
    }
  }

  bool bResult = m_BoundSRVs.RemoveAndCopy(desc);
  EZ_ASSERT_DEV(bResult, "The bound texture was not found.");

  m_BoundSRVs.Compact();
}

void spCommandListD3D11::UnbindUAVTexture(const spTextureViewDescription& desc)
{
  for (ezUInt32 i = 0, l = m_BoundUAVs.GetCount(); i < l; ++i)
  {
    if (m_BoundUAVs.GetKey(i) == desc)
    {
      const BoundTextureInfo& info = m_BoundUAVs.GetValue(i);
      BindUnorderedAccessView(nullptr, nullptr, nullptr, info.m_uiSlot, info.m_eStages, info.m_uiResourceSet);

      if (info.m_eStages.IsSet(spShaderStage::ComputeShader))
        m_InvalidatedComputeResourceSets[info.m_uiResourceSet] = true;
      else
        m_InvalidatedGraphicResourceSets[info.m_uiResourceSet] = true;
    }
  }

  bool bResult = m_BoundUAVs.RemoveAndCopy(desc);
  EZ_ASSERT_DEV(bResult, "The bound texture was not found.");

  m_BoundUAVs.Compact();
}

void spCommandListD3D11::UnbindUAVBuffer(const spBufferD3D11* pBuffer)
{
  UnbindUAVBufferPipeline(pBuffer, false);
  UnbindUAVBufferPipeline(pBuffer, true);
}

void spCommandListD3D11::UnbindUAVBufferPipeline(const spBufferD3D11* pBuffer, bool bCompute)
{
  auto& list = bCompute ? m_CachedComputeUAVBuffers : m_CachedGraphicUAVBuffers;
  for (ezUInt32 i = 0; i < list.GetCount(); ++i)
  {
    ezUInt32 uiSlot = list[i].second;

    if (bCompute)
      m_pImmediateContext->CSSetUnorderedAccessViews(uiSlot, 1, nullptr, nullptr);
    else
      m_pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, uiSlot, 1, nullptr, nullptr);

    list.RemoveAtAndSwap(i);
    --i;
  }
}

void spCommandListD3D11::PreDraw()
{
  FlushViewports();
  FlushScissorRects();
  FlushVertexBindings();

  ezUInt32 uiGraphicResourceCount = m_pGraphicPipeline->GetResourceLayouts().GetCount();
  for (ezUInt32 i = 0; i < uiGraphicResourceCount; ++i)
  {
    if (m_InvalidatedGraphicResourceSets[i])
    {
      m_InvalidatedGraphicResourceSets[i] = false;
      ActivateResourceSet(i, m_GraphicResourceSets[i], false);
    }
  }
}

void spCommandListD3D11::PreDispatch()
{
  ezUInt32 uiComputeResourceCount = m_pComputePipeline->GetResourceLayouts().GetCount();
  for (ezUInt32 i = 0; i < uiComputeResourceCount; ++i)
  {
    if (m_InvalidatedComputeResourceSets[i])
    {
      m_InvalidatedComputeResourceSets[i] = false;
      ActivateResourceSet(i, m_ComputeResourceSets[i], true);
    }
  }
}

void spCommandListD3D11::FlushViewports()
{
  if (!m_bViewportsChanged)
    return;

  m_bViewportsChanged = false;

  if (m_Viewports.IsEmpty())
    return;

  m_pImmediateContext->RSSetViewports(m_Viewports.GetCount(), m_Viewports.GetData());
}

void spCommandListD3D11::FlushScissorRects()
{
  if (!m_bScissorRectsChanged)
    return;

  m_bScissorRectsChanged = false;

  if (m_ScissorRects.IsEmpty())
    return;

  m_pImmediateContext->RSSetScissorRects(m_ScissorRects.GetCount(), m_ScissorRects.GetData());
}

void spCommandListD3D11::FlushVertexBindings()
{
  if (!m_bIsVertexBindingsDirty)
    return;

  m_bIsVertexBindingsDirty = false;

  if (m_uiNumVertexBuffers == 0)
    return;

  m_pImmediateContext->IASetVertexBuffers(0, m_uiNumVertexBuffers, m_VertexBuffers.GetData(), m_VertexStrides.GetData(), m_VertexOffsets.GetData());
}

void spCommandListD3D11::PackRangeParams(spBufferRangeD3D11* pBufferRange)
{
  m_ConstantBuffersOut[0] = pBufferRange->GetBuffer()->GetD3D11Buffer();
  m_FirstConstantBufferRef[0] = pBufferRange->GetOffset() / 16;
  ezUInt32 uiRoundedSize = pBufferRange->GetSize() < 256 ? 256u : pBufferRange->GetSize();
  m_ConstantBuffersRefCounts[0] = uiRoundedSize / 16;
}

void spCommandListD3D11::TrackBoundUAVBuffer(spBufferD3D11* pBuffer, ezUInt32 uiSlot, bool bCompute)
{
  if (bCompute)
    m_CachedComputeUAVBuffers.PushBack({pBuffer, uiSlot});
  else
    m_CachedGraphicUAVBuffers.PushBack({pBuffer, uiSlot});
}

void spCommandListD3D11::OnComplete()
{
  SP_RHI_DX11_RELEASE(m_pCommandList);

  for (auto it = m_ReferencedSwapchains.GetIterator(); it.IsValid(); it.Next())
    (*it)->RemoveCommandListReference(this);

  m_ReferencedSwapchains.Clear();

  for (auto it = m_SubmittedStagingBuffers.GetIterator(); it.IsValid(); it.Next())
    m_FreeBuffers.PushBack(*it);

  m_SubmittedStagingBuffers.Clear();
}
