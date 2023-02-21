#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/CommandList.h>

#include <RHID3D11/Buffer.h>
#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Framebuffer.h>
#include <RHID3D11/Pipeline.h>
#include <RHID3D11/Profiler.h>
#include <RHID3D11/ResourceLayout.h>
#include <RHID3D11/ResourceSet.h>
#include <RHID3D11/Sampler.h>
#include <RHID3D11/Shader.h>
#include <RHID3D11/Swapchain.h>
#include <RHID3D11/Texture.h>

// clang-format off
EZ_DEFINE_AS_POD_TYPE(D3D11_VIEWPORT);
EZ_DEFINE_AS_POD_TYPE(D3D11_RECT);

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spCommandListD3D11, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spCommandListD3D11::spCommandListD3D11(spDeviceD3D11* pDevice, const spCommandListDescription& description)
  : spCommandList(description)
{
  m_pDevice = pDevice;

  const HRESULT res = pDevice->GetD3D11Device()->CreateDeferredContext3(0, &m_pDeviceContext);
  EZ_HRESULT_TO_ASSERT(res);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (FAILED(m_pDeviceContext->QueryInterface(IID_ID3DUserDefinedAnnotation, reinterpret_cast<void**>(&m_pUserDefinedAnnotation))))
    ezLog::Warning("Could not get ID3DUserDefinedAnnotation interface, some features may not be available.");
#endif
  m_bReleased = false;
}

spCommandListD3D11::~spCommandListD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

void spCommandListD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  Reset();

  SP_RHI_DX11_RELEASE(m_pCommandList);
  SP_RHI_DX11_RELEASE(m_pUserDefinedAnnotation);
  SP_RHI_DX11_RELEASE(m_pDeviceContext);

  for (auto& set : m_GraphicResourceSets)
    set.m_Offsets.Clear();

  for (auto& set : m_ComputeResourceSets)
    set.m_Offsets.Clear();

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
  m_pDeviceContext->Dispatch(uiGroupCountX, uiGroupCountY, uiGroupCountZ);
}

void spCommandListD3D11::SetComputePipeline(ezSharedPtr<spComputePipeline> pComputePipeline)
{
  EZ_ASSERT_DEV(pComputePipeline != nullptr, "Invalid compute pipeline handle");

  ClearSets(m_ComputeResourceSets);
  m_InvalidatedComputeResourceSets.Clear();

  m_pComputeShader = pComputePipeline.Downcast<spComputePipelineD3D11>()->GetComputeShader();
  m_pDeviceContext->CSSetShader(m_pComputeShader, nullptr, 0);

  m_ComputeResourceSets.EnsureCount(pComputePipeline->GetResourceLayouts().GetCount());
  m_InvalidatedComputeResourceSets.EnsureCount(pComputePipeline->GetResourceLayouts().GetCount());

  m_pComputePipeline = pComputePipeline;
}

void spCommandListD3D11::SetGraphicPipeline(ezSharedPtr<spGraphicPipeline> pGraphicPipeline)
{
  EZ_ASSERT_DEV(pGraphicPipeline != nullptr, "Invalid graphic pipeline handle");

  const auto pGraphicPipelineD3D11 = pGraphicPipeline.Downcast<spGraphicPipelineD3D11>();

  ClearSets(m_GraphicResourceSets);
  m_InvalidatedGraphicResourceSets.Clear();

  ID3D11BlendState* pBlendState = pGraphicPipelineD3D11->GetBlendState();
  const ezColor blendFactor = pGraphicPipelineD3D11->GetBlendFactor();
  if (m_pBlendState != pBlendState || m_BlendFactor != blendFactor)
  {
    m_pBlendState = pBlendState;
    m_BlendFactor = blendFactor;
    m_pDeviceContext->OMSetBlendState(m_pBlendState, blendFactor.GetData(), 0xFFFFFFFFu);
  }

  ID3D11DepthStencilState* pDepthStencilState = pGraphicPipelineD3D11->GetDepthStencilState();
  const ezUInt32 uiStencilRef = pGraphicPipelineD3D11->GetStencilRef();
  if (m_pDepthStencilState != pDepthStencilState || m_uiStencilRef != uiStencilRef)
  {
    m_pDepthStencilState = pDepthStencilState;
    m_uiStencilRef = uiStencilRef;
    m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState, uiStencilRef);
  }

  if (ID3D11RasterizerState* pRasterizerState = pGraphicPipelineD3D11->GetRasterizerState(); m_pRasterizerState != pRasterizerState)
  {
    m_pRasterizerState = pRasterizerState;
    m_pDeviceContext->RSSetState(m_pRasterizerState);
  }

  if (const D3D11_PRIMITIVE_TOPOLOGY ePrimitiveTopology = pGraphicPipelineD3D11->GetPrimitiveTopology(); m_ePrimitiveTopology != ePrimitiveTopology)
  {
    m_ePrimitiveTopology = ePrimitiveTopology;
    m_pDeviceContext->IASetPrimitiveTopology(m_ePrimitiveTopology);
  }

  if (ID3D11InputLayout* pInputLayout = pGraphicPipelineD3D11->GetInputLayout(); m_pInputLayout != pInputLayout)
  {
    m_pInputLayout = pInputLayout;
    m_pDeviceContext->IASetInputLayout(m_pInputLayout);
  }

  if (ID3D11VertexShader* pVertexShader = pGraphicPipelineD3D11->GetVertexShader(); m_pVertexShader != pVertexShader)
  {
    m_pVertexShader = pVertexShader;
    m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
  }

  if (ID3D11GeometryShader* pGeometryShader = pGraphicPipelineD3D11->GetGeometryShader(); m_pGeometryShader != pGeometryShader)
  {
    m_pGeometryShader = pGeometryShader;
    m_pDeviceContext->GSSetShader(m_pGeometryShader, nullptr, 0);
  }

  if (ID3D11HullShader* pHullShader = pGraphicPipelineD3D11->GetHullShader(); m_pHullShader != pHullShader)
  {
    m_pHullShader = pHullShader;
    m_pDeviceContext->HSSetShader(m_pHullShader, nullptr, 0);
  }

  if (ID3D11DomainShader* pDomainShader = pGraphicPipelineD3D11->GetDomainShader(); m_pDomainShader != pDomainShader)
  {
    m_pDomainShader = pDomainShader;
    m_pDeviceContext->DSSetShader(m_pDomainShader, nullptr, 0);
  }

  if (ID3D11PixelShader* pPixelShader = pGraphicPipelineD3D11->GetPixelShader(); m_pPixelShader != pPixelShader)
  {
    m_pPixelShader = pPixelShader;
    m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
  }

  m_VertexStrides = pGraphicPipelineD3D11->GetVertexStrides();
  if (!m_VertexStrides.IsEmpty())
  {
    const ezUInt32 uiVertexStridesCount = m_VertexStrides.GetCount();
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

  m_ScissorRects[uiSlot].left = static_cast<LONG>(uiX);
  m_ScissorRects[uiSlot].right = static_cast<LONG>(uiX) + static_cast<LONG>(uiWidth);
  m_ScissorRects[uiSlot].top = static_cast<LONG>(uiY);
  m_ScissorRects[uiSlot].bottom = static_cast<LONG>(uiY) + static_cast<LONG>(uiHeight);
}

void spCommandListD3D11::SetViewport(ezUInt32 uiSlot, const spViewport& viewport)
{
  m_bViewportsChanged = true;
  m_Viewports.EnsureCount(uiSlot + 1);

  m_Viewports[uiSlot].TopLeftX = static_cast<FLOAT>(viewport.m_iX);
  m_Viewports[uiSlot].TopLeftY = static_cast<FLOAT>(viewport.m_iY);
  m_Viewports[uiSlot].Width = static_cast<FLOAT>(viewport.m_uiWidth);
  m_Viewports[uiSlot].Height = static_cast<FLOAT>(viewport.m_uiHeight);
  m_Viewports[uiSlot].MinDepth = viewport.m_fMinDepth;
  m_Viewports[uiSlot].MaxDepth = viewport.m_fMaxDepth;
}

void spCommandListD3D11::PushProfileScope(ezStringView sName)
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  m_pCurrentScopeProfiler = EZ_NEW(m_pDevice->GetAllocator(), spScopeProfilerD3D11, ezStaticCast<spDeviceD3D11*>(m_pDevice));
  m_pCurrentScopeProfiler->Begin(sName, m_pDeviceContext);
#endif

  PushDebugGroup(sName);
}

void spCommandListD3D11::PopProfileScope(ezSharedPtr<spScopeProfiler>& scopeProfiler)
{
  PopDebugGroup();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (m_pCurrentScopeProfiler != nullptr)
  {
    m_pCurrentScopeProfiler->End(m_pDeviceContext);
    scopeProfiler = m_pCurrentScopeProfiler;
  }

  m_pCurrentScopeProfiler.Clear();
#endif
}

void spCommandListD3D11::PushDebugGroup(ezStringView sName)
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (m_pUserDefinedAnnotation == nullptr)
    return;

  const ezStringWChar wsMarker(sName);
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

void spCommandListD3D11::InsertDebugMarker(ezStringView sName)
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (m_pUserDefinedAnnotation == nullptr)
    return;

  const ezStringWChar wsMarker(sName);
  m_pUserDefinedAnnotation->SetMarker(wsMarker.GetData());
#endif
}

void spCommandListD3D11::End()
{
  EZ_ASSERT_DEV(m_pCommandList == nullptr, "Invalid call to End");

  const HRESULT res = m_pDeviceContext->FinishCommandList(false, &m_pCommandList);
  EZ_HRESULT_TO_ASSERT(res);

  m_pCommandList->SetPrivateData(WKPDID_D3DDebugObjectName, m_sDebugName.GetElementCount(), m_sDebugName.GetStartPointer());

  ResetManagedState();
  m_bHasStarted = false;
}

void spCommandListD3D11::ClearColorTargetInternal(ezUInt32 uiIndex, ezColor clearColor)
{
  m_pDeviceContext->ClearRenderTargetView(m_pFramebuffer->GetRenderTargetView(uiIndex), clearColor.GetData());
}

void spCommandListD3D11::ClearDepthStencilTargetInternal(float fClearDepth, ezUInt8 uiClearStencil)
{
  m_pDeviceContext->ClearDepthStencilView(m_pFramebuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, fClearDepth, uiClearStencil);
}

void spCommandListD3D11::DrawInternal(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart)
{
  PreDraw();

  if (uiInstanceCount == 1 && uiInstanceStart == 0)
    m_pDeviceContext->Draw(uiVertexCount, uiVertexStart);
  else
    m_pDeviceContext->DrawInstanced(uiVertexCount, uiInstanceCount, uiVertexStart, uiInstanceStart);
}

void spCommandListD3D11::DrawIndexedInternal(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart)
{
  PreDraw();

  EZ_ASSERT_DEV(m_pIndexBuffer != nullptr, "Index buffer is not set.");

  if (uiInstanceCount == 1 && uiInstanceStart == 0)
    m_pDeviceContext->DrawIndexed(uiIndexCount, uiIndexStart, static_cast<INT>(uiVertexOffset));
  else
    m_pDeviceContext->DrawIndexedInstanced(uiIndexCount, uiInstanceCount, uiIndexStart, static_cast<INT>(uiVertexOffset), uiInstanceStart);
}

void spCommandListD3D11::DrawIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
{
  PreDraw();

  const auto pBufferD3D11 = pIndirectBuffer.Downcast<spBufferD3D11>();
  pBufferD3D11->EnsureResourceCreated();

  ezUInt32 uiCurrentOffset = uiOffset;

  for (ezUInt32 i = 0; i < uiDrawCount; i++)
  {
    m_pDeviceContext->DrawInstancedIndirect(pBufferD3D11->GetD3D11Buffer(), uiCurrentOffset);
    uiCurrentOffset += uiStride;
  }
}

void spCommandListD3D11::DrawIndexedIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
{
  PreDraw();

  const auto pBufferD3D11 = pIndirectBuffer.Downcast<spBufferD3D11>();
  pBufferD3D11->EnsureResourceCreated();

  ezUInt32 uiCurrentOffset = uiOffset;

  for (ezUInt32 i = 0; i < uiDrawCount; i++)
  {
    m_pDeviceContext->DrawIndexedInstancedIndirect(pBufferD3D11->GetD3D11Buffer(), uiCurrentOffset);
    uiCurrentOffset += uiStride;
  }
}

void spCommandListD3D11::DispatchIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset)
{
  PreDispatch();

  const auto pBufferD3D11 = pIndirectBuffer.Downcast<spBufferD3D11>();
  pBufferD3D11->EnsureResourceCreated();

  m_pDeviceContext->DispatchIndirect(pBufferD3D11->GetD3D11Buffer(), uiOffset);
}

void spCommandListD3D11::ResolveTextureInternal(ezSharedPtr<spTexture> pSource, ezSharedPtr<spTexture> pDestination)
{
  const auto pSourceD3D11 = pSource.Downcast<spTextureD3D11>();
  const auto pDestinationD3D11 = pDestination.Downcast<spTextureD3D11>();

  pSourceD3D11->EnsureResourceCreated();
  pDestinationD3D11->EnsureResourceCreated();

  m_pDeviceContext->ResolveSubresource(pSourceD3D11->GetD3D11Texture(), 0, pDestinationD3D11->GetD3D11Texture(), 0, pDestinationD3D11->GetDXGIFormat());
}

void spCommandListD3D11::SetFramebufferInternal(ezSharedPtr<spFramebuffer> pFramebuffer)
{
  // Borrowing the pointer here since it's already referenced by the parent command list
  m_pFramebuffer = pFramebuffer.Downcast<spFramebufferD3D11>();
  m_pFramebuffer->EnsureResourceCreated();

  if (const auto pSwapchain = m_pFramebuffer->GetParentSwapchain(); pSwapchain != nullptr)
  {
    pSwapchain->AddCommandListReference(this);
    m_ReferencedSwapchainList.PushBack(pSwapchain);
  }

  for (ezUInt32 i = 0, l = m_pFramebuffer->GetColorTargets().GetCount(); i < l; ++i)
  {
    UnbindSRVTexture(spTextureViewDescription(m_pFramebuffer->GetColorTarget(i)->GetHandle()));
  }

  const auto& views = m_pFramebuffer->GetRenderTargetViews();
  m_pDeviceContext->OMSetRenderTargets(views.GetCount(), views.GetPtr(), m_pFramebuffer->GetDepthStencilView());
}

void spCommandListD3D11::SetIndexBufferInternal(ezSharedPtr<spBuffer> pIndexBuffer, ezEnum<spIndexFormat> eFormat, ezUInt32 uiOffset)
{
  const auto pIndexBufferD3D11 = pIndexBuffer.Downcast<spBufferD3D11>();
  pIndexBufferD3D11->EnsureResourceCreated();

  if (m_pIndexBuffer != pIndexBufferD3D11 || m_uiIndexBufferOffset != uiOffset)
  {
    m_pIndexBuffer = pIndexBufferD3D11;
    m_uiIndexBufferOffset = uiOffset;

    UnbindUAVBuffer(pIndexBufferD3D11);
    m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer->GetD3D11Buffer(), spToD3D11(eFormat), uiOffset);
  }
}

void spCommandListD3D11::SetComputeResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
{
  spCommandListResourceSet& set = m_ComputeResourceSets[uiSlot];

  if (set.m_hResourceSet == pResourceSet->GetHandle() && ezMemoryUtils::IsEqual(set.m_Offsets.GetPtr(), pDynamicOffsets, uiDynamicOffsetCount))
    return;

  set.m_Offsets.Clear();
  set = spCommandListResourceSet(pResourceSet->GetHandle(), uiDynamicOffsetCount, pDynamicOffsets);
  ActivateResourceSet(uiSlot, set, true);
}

void spCommandListD3D11::SetGraphicResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
{
  spCommandListResourceSet& set = m_GraphicResourceSets[uiSlot];

  if (set.m_hResourceSet == pResourceSet->GetHandle() && ezMemoryUtils::IsEqual(set.m_Offsets.GetPtr(), pDynamicOffsets, uiDynamicOffsetCount))
    return;

  set.m_Offsets.Clear();
  set = spCommandListResourceSet(pResourceSet->GetHandle(), uiDynamicOffsetCount, pDynamicOffsets);
  ActivateResourceSet(uiSlot, set, false);
}

void spCommandListD3D11::SetVertexBufferInternal(ezUInt32 uiSlot, ezSharedPtr<spBuffer> pVertexBuffer, ezUInt32 uiOffset)
{
  const auto pBuffer = pVertexBuffer.Downcast<spBufferD3D11>();
  pBuffer->EnsureResourceCreated();

  if (m_VertexBuffers[uiSlot] != pBuffer->GetD3D11Buffer() || m_VertexOffsets[uiSlot] != uiOffset)
  {
    m_bIsVertexBindingsDirty = true;
    UnbindUAVBuffer(pBuffer);

    m_VertexBuffers[uiSlot] = pBuffer->GetD3D11Buffer();
    m_VertexOffsets[uiSlot] = uiOffset;

    m_uiNumVertexBuffers = ezMath::Max(m_uiNumVertexBuffers, uiSlot + 1);
  }
}

void spCommandListD3D11::UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize)
{
  const auto pBufferD3D11 = pBuffer.Downcast<spBufferD3D11>();
  pBufferD3D11->EnsureResourceCreated();

  const auto pBufferRange = pBuffer->GetCurrentBuffer();
  uiOffset += pBufferRange->GetOffset();

  const bool bDynamic = pBuffer->GetUsage().IsSet(spBufferUsage::Dynamic);
  const bool bStaging = pBuffer->GetUsage().IsSet(spBufferUsage::Staging);
  const bool bConstantBuffer = pBuffer->GetUsage().IsSet(spBufferUsage::ConstantBuffer);
  const bool bUseMap = bDynamic;
  const bool bUpdateFullBuffer = uiOffset == 0 && uiSize == pBufferD3D11->GetSize();

  if (!bDynamic && !bStaging && (!bConstantBuffer || bUpdateFullBuffer))
  {
    const D3D11_BOX region = {uiOffset, 0, 0, uiOffset + uiSize, 1, 1};
    const bool bHasRegion = !bConstantBuffer;

    if (uiOffset == 0)
    {
      m_pDeviceContext->UpdateSubresource(pBufferD3D11->GetD3D11Buffer(), 0, bHasRegion ? &region : nullptr, pSourceData, 0, 0);
    }
    else
    {
      const bool bNeedWorkAround = !m_pDevice->GetCapabilities().m_bSupportCommandLists;
      const void* pAdjustedData = pSourceData;

      if (bNeedWorkAround)
      {
        EZ_ASSERT_DEV(region.top == 0 && region.front == 0, "Invalid region.");
        pAdjustedData = static_cast<const ezUInt8*>(pSourceData) - region.left;
      }

      m_pDeviceContext->UpdateSubresource(pBufferD3D11->GetD3D11Buffer(), 0, &region, pAdjustedData, 0, 0);
    }
  }
  else if (bUseMap && bUpdateFullBuffer) // Can only update full buffer with WriteDiscard
  {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_pDeviceContext->Map(pBufferD3D11->GetD3D11Buffer(), 0, spToD3D11(spMapAccess::Write, bDynamic), 0, &mappedResource);

    ezMemoryUtils::RawByteCopy(mappedResource.pData, pSourceData, uiSize);

    m_pDeviceContext->Unmap(pBufferD3D11->GetD3D11Buffer(), 0);
  }
  else
  {
    const ezSharedPtr<spBufferD3D11> pStagingBufferD3D11 = GetFreeStagingBuffer(uiSize);
    pStagingBufferD3D11->EnsureResourceCreated();

    m_pDevice->UpdateBuffer(pStagingBufferD3D11, 0, pSourceData, uiSize);
    CopyBuffer(pStagingBufferD3D11, 0, pBuffer, uiOffset, uiSize);
    m_SubmittedStagingBuffers.PushBack(pStagingBufferD3D11);
  }
}

void spCommandListD3D11::CopyBufferInternal(ezSharedPtr<spBuffer> pSourceBuffer, ezUInt32 uiSourceOffset, ezSharedPtr<spBuffer> pDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize)
{
  const auto pDestBufferD3D11 = pDestBuffer.Downcast<spBufferD3D11>();
  const auto pSourceBufferD3D11 = pSourceBuffer.Downcast<spBufferD3D11>();

  pSourceBufferD3D11->EnsureResourceCreated();
  pDestBufferD3D11->EnsureResourceCreated();

  const D3D11_BOX srcBox = {uiSourceOffset, 0, 0, uiSourceOffset + uiSize, 1, 1};
  m_pDeviceContext->CopySubresourceRegion(pDestBufferD3D11->GetD3D11Buffer(), 0, uiDestOffset, 0, 0, pSourceBufferD3D11->GetD3D11Buffer(), uiSourceOffset, &srcBox);
}

void spCommandListD3D11::CopyTextureInternal(ezSharedPtr<spTexture> pSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, ezSharedPtr<spTexture> pDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount)
{
  const auto pSourceTextureD3D11 = pSourceTexture.Downcast<spTextureD3D11>();
  const auto pDestinationTextureD3D11 = pDestinationTexture.Downcast<spTextureD3D11>();

  pSourceTextureD3D11->EnsureResourceCreated();
  pDestinationTextureD3D11->EnsureResourceCreated();

  const ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(pSourceTexture->GetFormat()) ? 4u : 1u;
  const ezUInt32 uiClampedWidth = ezMath::Max(uiWidth, uiBlockSize);
  const ezUInt32 uiClampedHeight = ezMath::Max(uiHeight, uiBlockSize);

  D3D11_BOX region;
  bool bHasRegion = false;
  if (uiSourceX != 0 || uiSourceY != 0 || uiSourceZ != 0 || uiClampedWidth != pSourceTextureD3D11->GetWidth() || uiClampedHeight != pSourceTextureD3D11->GetHeight() || uiDepth != pSourceTextureD3D11->GetDepth())
  {
    bHasRegion = true;
    region = {uiSourceX, uiSourceY, uiSourceZ, uiClampedWidth + uiSourceX, uiClampedHeight + uiSourceY, uiDepth + uiSourceZ};
  }

  for (ezUInt32 i = 0; i < uiLayerCount; ++i)
  {
    const ezUInt32 uiSrcSubresource = spTextureHelper::CalculateSubresource(pSourceTexture, uiSourceMipLevel, uiSourceBaseArrayLayer + i);
    const ezUInt32 uiDestSubresource = spTextureHelper::CalculateSubresource(pDestinationTexture, uiDestMipLevel, uiDestBaseArrayLayer + i);

    m_pDeviceContext->CopySubresourceRegion(pDestinationTextureD3D11->GetD3D11Texture(), uiDestSubresource, uiDestX, uiDestY, uiDestZ, pSourceTextureD3D11->GetD3D11Texture(), uiSrcSubresource, bHasRegion ? &region : nullptr);
  }
}

void spCommandListD3D11::GenerateMipmapsInternal(ezSharedPtr<spTexture> pTexture)
{
  const auto pTextureViewD3D11 = m_pDevice->GetTextureSamplerManager()->GetFullTextureView(pTexture).Downcast<spTextureViewD3D11>();
  pTextureViewD3D11->EnsureResourceCreated();

  ID3D11ShaderResourceView* pSRV = pTextureViewD3D11->GetShaderResourceView();
  m_pDeviceContext->GenerateMips(pSRV);
}

void spCommandListD3D11::Reset()
{
  ClearCachedState();

  if (m_bHasStarted)
  {
    m_pDeviceContext->ClearState();
    m_pDeviceContext->FinishCommandList(false, &m_pCommandList);
  }

  SP_RHI_DX11_RELEASE(m_pCommandList);

  ResetManagedState();
  m_bHasStarted = false;
}

void spCommandListD3D11::ClearState()
{
  ClearCachedState();
  m_pDeviceContext->ClearState();
  ResetManagedState();
}

void spCommandListD3D11::ResetManagedState()
{
  m_pCurrentScopeProfiler.Clear();

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

  m_pGraphicPipeline = nullptr;
  ClearSets(m_GraphicResourceSets);

  m_CachedVertexConstantBuffers.Clear();
  m_CachedVertexTextureViews.Clear();
  m_CachedVertexSamplerStates.Clear();

  m_CachedPixelConstantBuffers.Clear();
  m_CachedPixelTextureViews.Clear();
  m_CachedPixelSamplerStates.Clear();

  m_CachedGraphicUAVBuffers.Clear();
  m_CachedComputeUAVBuffers.Clear();

  m_pComputePipeline = nullptr;
  ClearSets(m_ComputeResourceSets);

  m_BoundSRVs.Clear();
  m_BoundUAVs.Clear();
}

void spCommandListD3D11::ClearSets(ezDynamicArray<spCommandListResourceSet>& sets)
{
  for (ezUInt32 i = 0, l = sets.GetCount(); i < l; ++i)
    sets[i].m_Offsets.Clear();

  sets.Clear();
}

void spCommandListD3D11::ActivateResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet, bool bCompute)
{
  const ezSharedPtr<spResourceSetD3D11> pResourceSet = m_pDevice->GetResourceManager()->GetResource<spResourceSetD3D11>(resourceSet.m_hResourceSet);
  EZ_ASSERT_DEV(pResourceSet != nullptr, "Invalid resource set handle");

  const ezUInt32 uiConstantBufferBase = GetConstantBufferBase(uiSlot, bCompute);
  const ezUInt32 uiUnorderedAccessViewBase = GetUnorderedAccessViewBase(uiSlot, bCompute);
  const ezUInt32 uiTextureBase = GetTextureBase(uiSlot, bCompute);
  const ezUInt32 uiSamplerBase = GetSamplerBase(uiSlot, bCompute);

  const ezSharedPtr<spResourceLayoutD3D11> pLayout = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutD3D11>(pResourceSet->GetLayout());
  EZ_ASSERT_DEV(pLayout != nullptr, "Invalid resource layout handle");

  const auto& resources = pResourceSet->GetResources();
  ezUInt32 uiDynamicOffsetIndex = 0;

  for (ezUInt32 i = 0, l = resources.GetCount(); i < l; ++i)
  {
    const ezSharedPtr<spShaderResource> pResource = resources[i];
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
        const ezSharedPtr<spBufferRangeD3D11> pBufferRange = GetBufferRange(pResource, uiBufferOffset);
        BindConstantBuffer(pBufferRange, uiConstantBufferBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage);
        break;
      }
      case spShaderResourceType::ReadOnlyStructuredBuffer:
      {
        const ezSharedPtr<spBufferRangeD3D11> pBufferRange = GetBufferRange(pResource, uiBufferOffset);
        BindStorageBufferView(pBufferRange, uiTextureBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage);
        break;
      }
      case spShaderResourceType::ReadWriteStructuredBuffer:
      {
        const ezSharedPtr<spBufferRangeD3D11> pBufferRange = GetBufferRange(pResource, uiBufferOffset);
        ID3D11UnorderedAccessView* pUAV = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetUnorderedAccessView(pBufferRange->GetOffset(), pBufferRange->GetSize());
        BindUnorderedAccessView(nullptr, pBufferRange->GetBuffer().Downcast<spBufferD3D11>(), pUAV, uiUnorderedAccessViewBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage, uiSlot);
        break;
      }
      case spShaderResourceType::ReadOnlyTexture:
      {
        const ezSharedPtr<spTextureViewD3D11> pTextureView = spTextureSamplerManager::GetTextureView(m_pDevice, pResource).Downcast<spTextureViewD3D11>();
        UnbindUAVTexture(pTextureView->GetDescription());
        BindTextureView(pTextureView, uiTextureBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage, uiSlot);
        break;
      }
      case spShaderResourceType::ReadWriteTexture:
      {
        const ezSharedPtr<spTextureViewD3D11> pTextureView = spTextureSamplerManager::GetTextureView(m_pDevice, pResource).Downcast<spTextureViewD3D11>();
        const ezSharedPtr<spTextureD3D11> pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(pTextureView->GetTexture());
        UnbindSRVTexture(pTextureView->GetDescription());
        BindUnorderedAccessView(pTexture, nullptr, pTextureView->GetUnorderedAccessView(), uiUnorderedAccessViewBase + layoutBindingInfo.m_uiSlot, layoutBindingInfo.m_eShaderStage, uiSlot);
        break;
      }
      case spShaderResourceType::Sampler:
      {
        const ezSharedPtr<spSamplerD3D11> pSampler = pResource.Downcast<spSamplerD3D11>();
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

ezSharedPtr<spBufferD3D11> spCommandListD3D11::GetFreeStagingBuffer(ezUInt32 uiSize)
{
  for (auto it = m_FreeBuffers.GetIterator(); it.IsValid(); it.Next())
  {
    if ((*it)->GetSize() >= uiSize)
    {
      ezSharedPtr<spBufferD3D11> pBuffer = *it;
      m_FreeBuffers.Remove(it);

      return pBuffer;
    }
  }

  return m_pDevice->GetResourceFactory()->CreateBuffer(spBufferDescription(uiSize, spBufferUsage::Staging)).Downcast<spBufferD3D11>();
}

ezUInt32 spCommandListD3D11::GetConstantBufferBase(ezUInt32 uiSlot, bool bCompute) const
{
  const auto layouts = bCompute ? m_pComputePipeline.Downcast<spComputePipelineD3D11>()->GetResourceLayouts().GetArrayPtr() : m_pGraphicPipeline.Downcast<spGraphicPipelineD3D11>()->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i].Downcast<spResourceLayoutD3D11>()->GetStorageBufferCount();
  }

  return uiBase;
}

ezUInt32 spCommandListD3D11::GetUnorderedAccessViewBase(ezUInt32 uiSlot, bool bCompute) const
{
  const auto layouts = bCompute ? m_pComputePipeline.Downcast<spComputePipelineD3D11>()->GetResourceLayouts().GetArrayPtr() : m_pGraphicPipeline.Downcast<spGraphicPipelineD3D11>()->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i].Downcast<spResourceLayoutD3D11>()->GetStorageBufferCount();
  }

  return uiBase;
}

ezUInt32 spCommandListD3D11::GetTextureBase(ezUInt32 uiSlot, bool bCompute) const
{
  const auto layouts = bCompute ? m_pComputePipeline.Downcast<spComputePipelineD3D11>()->GetResourceLayouts().GetArrayPtr() : m_pGraphicPipeline.Downcast<spGraphicPipelineD3D11>()->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i].Downcast<spResourceLayoutD3D11>()->GetTextureCount();
  }

  return uiBase;
}

ezUInt32 spCommandListD3D11::GetSamplerBase(ezUInt32 uiSlot, bool bCompute) const
{
  const auto layouts = bCompute ? m_pComputePipeline.Downcast<spComputePipelineD3D11>()->GetResourceLayouts().GetArrayPtr() : m_pGraphicPipeline.Downcast<spGraphicPipelineD3D11>()->GetResourceLayouts().GetArrayPtr();
  ezUInt32 uiBase = 0;

  for (ezUInt32 i = 0; i < uiSlot; ++i)
  {
    EZ_ASSERT_DEV(layouts[i] != nullptr, "Invalid layout at slot {0}", i);
    uiBase += layouts[i].Downcast<spResourceLayoutD3D11>()->GetSamplerCount();
  }

  return uiBase;
}

ezSharedPtr<spBufferRangeD3D11> spCommandListD3D11::GetBufferRange(ezSharedPtr<spShaderResource> pResource, ezUInt32 uiOffset) const
{
  ezSharedPtr<spBufferRangeD3D11> pBufferRangeSource = nullptr;

  if (pResource->IsInstanceOf<spBufferRangeD3D11>())
  {
    pBufferRangeSource = pResource.Downcast<spBufferRangeD3D11>();
  }
  else if (pResource->IsInstanceOf<spBufferD3D11>())
  {
    const auto pBufferD3D11 = pResource.Downcast<spBufferD3D11>();
    pBufferD3D11->EnsureResourceCreated();

    pBufferRangeSource = pBufferD3D11->GetCurrentBuffer().Downcast<spBufferRangeD3D11>();
  }

  return m_pDevice->GetResourceFactory()->CreateBufferRange(spBufferRangeDescription(pBufferRangeSource->GetBuffer()->GetHandle(), uiOffset + pBufferRangeSource->GetOffset(), pBufferRangeSource->GetSize())).Downcast<spBufferRangeD3D11>();
}

void spCommandListD3D11::BindConstantBuffer(ezSharedPtr<spBufferRangeD3D11> pBufferRange, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages)
{
  if (eStages.IsSet(spShaderStage::VertexShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedConstantBuffers)
    {
      if (uiSlot >= m_CachedVertexConstantBuffers.GetCount())
      {
        m_CachedVertexConstantBuffers.ExpandAndGetRef() = pBufferRange;
        bBind = true;
      }
      else if (m_CachedVertexConstantBuffers[uiSlot] != pBufferRange)
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
        ID3D11Buffer* pBuffer = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetD3D11Buffer();
        m_pDeviceContext->VSSetConstantBuffers(uiSlot, 1, &pBuffer);
      }
      else
      {
        PackRangeParams(pBufferRange);
        if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
          m_pDeviceContext->VSSetConstantBuffers(uiSlot, 1, nullptr);

        m_pDeviceContext->VSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
      }
    }
  }

  if (eStages.IsSet(spShaderStage::GeometryShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetD3D11Buffer();
      m_pDeviceContext->GSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pDeviceContext->GSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pDeviceContext->GSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }

  if (eStages.IsSet(spShaderStage::HullShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetD3D11Buffer();
      m_pDeviceContext->HSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pDeviceContext->HSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pDeviceContext->HSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }

  if (eStages.IsSet(spShaderStage::DomainShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetD3D11Buffer();
      m_pDeviceContext->DSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pDeviceContext->DSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pDeviceContext->DSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }

  if (eStages.IsSet(spShaderStage::PixelShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedConstantBuffers)
    {
      if (uiSlot >= m_CachedPixelConstantBuffers.GetCount())
      {
        m_CachedPixelConstantBuffers.ExpandAndGetRef() = pBufferRange;
        bBind = true;
      }
      else if (m_CachedPixelConstantBuffers[uiSlot] != pBufferRange)
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
        ID3D11Buffer* pBuffer = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetD3D11Buffer();
        m_pDeviceContext->PSSetConstantBuffers(uiSlot, 1, &pBuffer);
      }
      else
      {
        PackRangeParams(pBufferRange);
        if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
          m_pDeviceContext->PSSetConstantBuffers(uiSlot, 1, nullptr);

        m_pDeviceContext->PSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
      }
    }
  }

  if (eStages.IsSet(spShaderStage::ComputeShader))
  {
    if (pBufferRange->IsFullRange())
    {
      ID3D11Buffer* pBuffer = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetD3D11Buffer();
      m_pDeviceContext->CSSetConstantBuffers(uiSlot, 1, &pBuffer);
    }
    else
    {
      PackRangeParams(pBufferRange);
      if (!m_pDevice->GetCapabilities().m_bSupportCommandLists)
        m_pDeviceContext->CSSetConstantBuffers(uiSlot, 1, nullptr);

      m_pDeviceContext->CSSetConstantBuffers1(uiSlot, 1, m_ConstantBuffersOut.GetData(), m_FirstConstantBufferRef.GetData(), m_ConstantBuffersRefCounts.GetData());
    }
  }
}

void spCommandListD3D11::BindStorageBufferView(ezSharedPtr<spBufferRangeD3D11> pBufferRange, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages)
{
  UnbindUAVBuffer(pBufferRange->GetBuffer().Downcast<spBufferD3D11>());

  ID3D11ShaderResourceView* pSRV = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetShaderResourceView(pBufferRange->GetOffset(), pBufferRange->GetSize());

  if (eStages.IsSet(spShaderStage::VertexShader))
    m_pDeviceContext->VSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::GeometryShader))
    m_pDeviceContext->GSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::HullShader))
    m_pDeviceContext->HSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::DomainShader))
    m_pDeviceContext->DSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::PixelShader))
    m_pDeviceContext->PSSetShaderResources(uiSlot, 1, &pSRV);

  if (eStages.IsSet(spShaderStage::ComputeShader))
    m_pDeviceContext->CSSetShaderResources(uiSlot, 1, &pSRV);
}

void spCommandListD3D11::BindTextureView(ezSharedPtr<spTextureViewD3D11> pTextureView, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages, ezUInt32 uiSetSlot)
{
  pTextureView->EnsureResourceCreated();

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
      if (uiSlot >= m_CachedVertexTextureViews.GetCount())
      {
        m_CachedVertexTextureViews.ExpandAndGetRef() = pTextureView;
        bBind = true;
      }
      else if (m_CachedVertexTextureViews[uiSlot] != pTextureView)
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
      m_pDeviceContext->VSSetShaderResources(uiSlot, 1, &pSRV);
    }
  }

  if (eStages.IsSet(spShaderStage::GeometryShader))
  {
    m_pDeviceContext->GSSetShaderResources(uiSlot, 1, &pSRV);
  }

  if (eStages.IsSet(spShaderStage::HullShader))
  {
    m_pDeviceContext->HSSetShaderResources(uiSlot, 1, &pSRV);
  }

  if (eStages.IsSet(spShaderStage::DomainShader))
  {
    m_pDeviceContext->DSSetShaderResources(uiSlot, 1, &pSRV);
  }

  if (eStages.IsSet(spShaderStage::PixelShader))
  {
    bool bBind = true;
    if (uiSlot < s_uiMaxCachedTextureViews)
    {
      if (uiSlot >= m_CachedPixelTextureViews.GetCount())
      {
        m_CachedPixelTextureViews.ExpandAndGetRef() = pTextureView;
        bBind = true;
      }
      else if (m_CachedPixelTextureViews[uiSlot] != pTextureView)
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
      m_pDeviceContext->PSSetShaderResources(uiSlot, 1, &pSRV);
    }
  }

  if (eStages.IsSet(spShaderStage::ComputeShader))
  {
    m_pDeviceContext->CSSetShaderResources(uiSlot, 1, &pSRV);
  }
}

void spCommandListD3D11::BindUnorderedAccessView(ezSharedPtr<spTextureD3D11> pTexture, ezSharedPtr<spBufferD3D11> pBuffer, ID3D11UnorderedAccessView* pUAV, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages, ezUInt32 uiSetSlot)
{
  pTexture->EnsureResourceCreated();

  const bool bCompute = eStages == spShaderStage::ComputeShader;
  EZ_ASSERT_DEV(bCompute || !eStages.IsSet(spShaderStage::ComputeShader), "Unordered access views cannot be bound on compute and graphic stages at the same time.");
  EZ_ASSERT_DEV(pTexture == nullptr || pBuffer == nullptr, "Cannot bind unordered access views to textures and buffers at the same time.");

  if (pTexture != nullptr && pUAV != nullptr)
  {
    BoundTextureInfo info{uiSlot, eStages, uiSetSlot};
    spTextureViewDescription desc(pTexture->GetHandle());
    m_BoundUAVs.Insert(desc, info);
  }

  ezUInt32 uiBaseSlot = 0;
  if (!bCompute && !m_CachedPixelSamplerStates.IsEmpty())
    uiBaseSlot = m_pFramebuffer->GetColorTargets().GetCount();

  const ezUInt32 uiActualSlot = uiBaseSlot + uiSlot;
  if (pBuffer != nullptr)
    TrackBoundUAVBuffer(pBuffer, uiActualSlot, bCompute);

  if (bCompute)
    m_pDeviceContext->CSSetUnorderedAccessViews(uiActualSlot, 1, &pUAV, nullptr);
  else
    m_pDeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, uiActualSlot, 1, &pUAV, nullptr);
}

void spCommandListD3D11::BindSampler(ezSharedPtr<spSamplerD3D11> pSampler, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages)
{
  pSampler->EnsureResourceCreated();

  ID3D11SamplerState* pD3D11SamplerState = pSampler->GetSamplerState()->GetD3D11SamplerState();

  if (eStages.IsSet(spShaderStage::VertexShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedSamplerStates)
    {
      if (uiSlot >= m_CachedVertexSamplerStates.GetCount())
      {
        m_CachedVertexSamplerStates.ExpandAndGetRef() = pSampler;
        bBind = true;
      }
      else if (m_CachedVertexSamplerStates[uiSlot] != pSampler)
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
      m_pDeviceContext->VSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
    }
  }

  if (eStages.IsSet(spShaderStage::GeometryShader))
  {
    m_pDeviceContext->GSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
  }

  if (eStages.IsSet(spShaderStage::HullShader))
  {
    m_pDeviceContext->HSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
  }

  if (eStages.IsSet(spShaderStage::DomainShader))
  {
    m_pDeviceContext->DSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
  }

  if (eStages.IsSet(spShaderStage::PixelShader))
  {
    bool bBind = false;
    if (uiSlot < s_uiMaxCachedSamplerStates)
    {
      if (uiSlot >= m_CachedPixelSamplerStates.GetCount())
      {
        m_CachedPixelSamplerStates.ExpandAndGetRef() = pSampler;
        bBind = true;
      }
      else if (m_CachedPixelSamplerStates[uiSlot] != pSampler)
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
      m_pDeviceContext->PSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
    }
  }

  if (eStages.IsSet(spShaderStage::ComputeShader))
  {
    m_pDeviceContext->CSSetSamplers(uiSlot, 1, &pD3D11SamplerState);
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

  while (m_BoundSRVs.RemoveAndCopy(desc))
  {
  }

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

  while (m_BoundUAVs.RemoveAndCopy(desc))
  {
  }

  m_BoundUAVs.Compact();
}

void spCommandListD3D11::UnbindUAVBuffer(ezSharedPtr<spBufferD3D11> pBuffer)
{
  UnbindUAVBufferPipeline(pBuffer, false);
  UnbindUAVBufferPipeline(pBuffer, true);
}

void spCommandListD3D11::UnbindUAVBufferPipeline(ezSharedPtr<spBufferD3D11> pBuffer, bool bCompute)
{
  auto& list = bCompute ? m_CachedComputeUAVBuffers : m_CachedGraphicUAVBuffers;
  for (ezUInt32 i = 0; i < list.GetCount(); ++i)
  {
    if (list[i].first == pBuffer)
    {
      const ezUInt32 uiSlot = list[i].second;

      if (bCompute)
        m_pDeviceContext->CSSetUnorderedAccessViews(uiSlot, 1, nullptr, nullptr);
      else
        m_pDeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, uiSlot, 1, nullptr, nullptr);

      list.RemoveAtAndSwap(i);
      --i;
    }
  }
}

void spCommandListD3D11::PreDraw()
{
  FlushViewports();
  FlushScissorRects();
  FlushVertexBindings();

  const ezUInt32 uiGraphicResourceCount = m_pGraphicPipeline->GetResourceLayouts().GetCount();
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
  const ezUInt32 uiComputeResourceCount = m_pComputePipeline->GetResourceLayouts().GetCount();
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

  m_pDeviceContext->RSSetViewports(m_Viewports.GetCount(), m_Viewports.GetData());
}

void spCommandListD3D11::FlushScissorRects()
{
  if (!m_bScissorRectsChanged)
    return;

  m_bScissorRectsChanged = false;

  if (m_ScissorRects.IsEmpty())
    return;

  m_pDeviceContext->RSSetScissorRects(m_ScissorRects.GetCount(), m_ScissorRects.GetData());
}

void spCommandListD3D11::FlushVertexBindings()
{
  if (!m_bIsVertexBindingsDirty)
    return;

  m_bIsVertexBindingsDirty = false;

  if (m_uiNumVertexBuffers == 0)
    return;

  m_pDeviceContext->IASetVertexBuffers(0, m_uiNumVertexBuffers, m_VertexBuffers.GetData(), m_VertexStrides.GetData(), m_VertexOffsets.GetData());
}

void spCommandListD3D11::PackRangeParams(ezSharedPtr<spBufferRangeD3D11> pBufferRange)
{
  m_ConstantBuffersOut.EnsureCount(1);
  m_FirstConstantBufferRef.EnsureCount(1);
  m_ConstantBuffersRefCounts.EnsureCount(1);

  m_ConstantBuffersOut[0] = pBufferRange->GetBuffer().Downcast<spBufferD3D11>()->GetD3D11Buffer();
  m_FirstConstantBufferRef[0] = pBufferRange->GetOffset() / 16;
  const ezUInt32 uiRoundedSize = pBufferRange->GetSize() < 256 ? 256u : pBufferRange->GetSize();
  m_ConstantBuffersRefCounts[0] = uiRoundedSize / 16;
}

void spCommandListD3D11::TrackBoundUAVBuffer(ezSharedPtr<spBufferD3D11> pBuffer, ezUInt32 uiSlot, bool bCompute)
{
  if (bCompute)
    m_CachedComputeUAVBuffers.PushBack({pBuffer, uiSlot});
  else
    m_CachedGraphicUAVBuffers.PushBack({pBuffer, uiSlot});
}

void spCommandListD3D11::OnComplete()
{
  SP_RHI_DX11_RELEASE(m_pCommandList);

  for (auto it = m_ReferencedSwapchainList.GetIterator(); it.IsValid(); it.Next())
  {
    (*it)->RemoveCommandListReference(this);
    (*it).Clear();
  }

  m_ReferencedSwapchainList.Clear();

  for (auto it = m_SubmittedStagingBuffers.GetIterator(); it.IsValid(); it.Next())
    m_FreeBuffers.PushBack(*it);

  m_SubmittedStagingBuffers.Clear();
}

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_CommandList);
