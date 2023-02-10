#include <RHI/RHIPCH.h>

#include <RHI/CommandList.h>

#include <RHI/Buffer.h>
#include <RHI/Device.h>
#include <RHI/Framebuffer.h>
#include <RHI/Output.h>
#include <RHI/Pipeline.h>
#include <RHI/ResourceLayout.h>
#include <RHI/ResourceSet.h>

#include <utility>

#pragma region spCommandListIndexBuffer

spCommandListIndexBuffer::spCommandListIndexBuffer(spResourceHandle hIndexBuffer, const ezEnum<spIndexFormat>& eFormat, ezUInt32 uiOffset)
  : ezHashableStruct<spCommandListIndexBuffer>()
  , m_hIndexBuffer(hIndexBuffer)
  , m_eIndexFormat(eFormat)
  , m_uiOffset(uiOffset)
{
}

#pragma endregion

#pragma region spCommandListResourceSet

spCommandListResourceSet::spCommandListResourceSet(spResourceHandle hResource, ezUInt32 uiOffsetCount, const ezUInt32* pOffsets)
  : ezHashableStruct<spCommandListResourceSet>()
  , m_hResourceSet(hResource)
{
  m_Offsets = EZ_DEFAULT_NEW_ARRAY(ezUInt32, uiOffsetCount);
  m_Offsets.CopyFrom(ezMakeArrayPtr(pOffsets, uiOffsetCount));
}

spCommandListResourceSet::~spCommandListResourceSet()
{
  EZ_DEFAULT_DELETE_ARRAY(m_Offsets);
}

#pragma endregion

#pragma region spCommandList

spCommandList::spCommandList(spCommandListDescription description)
  : spDeviceResource()
  , m_Description(std::move(description))
{
}

void spCommandList::ClearColorTarget(ezUInt32 uiIndex, ezColor clearColor)
{
  EZ_ASSERT_DEV(m_pFramebuffer != nullptr, "Cannot use ClearColorTarget without a framebuffer");
  EZ_ASSERT_DEV(m_pFramebuffer->GetColorTargets().GetCount() > uiIndex, "Index out of bounds. Values must be less than the number of color targets in the framebuffer.");

  ClearColorTargetInternal(uiIndex, clearColor);
}

void spCommandList::ClearDepthStencilTarget(float fClearDepth)
{
  ClearDepthStencilTarget(fClearDepth, 0);
}

void spCommandList::ClearDepthStencilTarget(float fClearDepth, ezUInt8 uiClearStencil)
{
  EZ_ASSERT_DEV(m_pFramebuffer != nullptr, "Cannot use ClearColorTarget without a framebuffer");
  EZ_ASSERT_DEV(!m_pFramebuffer->GetDepthTarget().IsInvalidated(), "The current framebuffer has no depth target attached.");

  ClearDepthStencilTargetInternal(fClearDepth, uiClearStencil);
}

void spCommandList::Draw(ezUInt32 uiVertexCount)
{
  Draw(uiVertexCount, 1, 0, 0);
}

void spCommandList::Draw(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart)
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  DrawValidation();
#endif

  DrawInternal(uiVertexCount, uiInstanceCount, uiVertexStart, uiInstanceStart);
}

void spCommandList::DrawIndexed(ezUInt32 uiIndexCount)
{
  DrawIndexed(uiIndexCount, 1, 0, 0, 0);
}

void spCommandList::DrawIndexed(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart)
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  IndexBufferValidation(uiIndexCount);
  DrawValidation();

  EZ_ASSERT_DEV(m_pDevice->GetCapabilities().m_bDrawBaseVertex || uiVertexOffset == 0, "Drawing with a non-zero base vertex is not supported on this device.");
  EZ_ASSERT_DEV(m_pDevice->GetCapabilities().m_bDrawBaseInstance || uiInstanceStart == 0, "Drawing with a non-zero base instance is not supported on this device.");
#endif

  DrawIndexedInternal(uiIndexCount, uiInstanceCount, uiIndexStart, uiVertexOffset, uiInstanceStart);
}

void spCommandList::DrawIndirect(spResourceHandle hIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
{
  auto pIndirectBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hIndirectBuffer);
  EZ_ASSERT_DEV(pIndirectBuffer != nullptr, "Invalid buffer handle.");

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  EZ_ASSERT_DEV(m_pDevice->GetCapabilities().m_bDrawIndirect, "Indirect drawing is not supported on this device.");
  EZ_ASSERT_DEV(pIndirectBuffer->GetUsage().IsSet(spBufferUsage::IndirectBuffer), "The buffer must have been created with the IndirectBuffer usage flag.");
  EZ_ASSERT_DEV(uiOffset % 4 == 0, "Offset must be aligned to 4 bytes");
  EZ_ASSERT_DEV(uiStride >= sizeof(spDrawIndirectCommand) && (uiStride % 4 == 0), "Stride must be aligned to 4 bytes and be larger than the size of the command structure.");
  DrawValidation();
#endif

  DrawIndirectInternal(pIndirectBuffer, uiOffset, uiDrawCount, uiStride);
}

void spCommandList::DrawIndexedIndirect(spResourceHandle hIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
{
  auto pIndirectBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hIndirectBuffer);
  EZ_ASSERT_DEV(pIndirectBuffer != nullptr, "Invalid buffer handle.");

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  EZ_ASSERT_DEV(m_pDevice->GetCapabilities().m_bDrawIndirect, "Indirect drawing is not supported on this device.");
  EZ_ASSERT_DEV(pIndirectBuffer->GetUsage().IsSet(spBufferUsage::IndirectBuffer), "The buffer must have been created with the IndirectBuffer usage flag.");
  EZ_ASSERT_DEV(uiOffset % 4 == 0, "Offset must be aligned to 4 bytes");
  EZ_ASSERT_DEV(uiStride >= sizeof(spDrawIndexedIndirectCommand) && (uiStride % 4 == 0), "Stride must be aligned to 4 bytes and be larger than the size of the command structure.");
  DrawValidation();
#endif

  DrawIndexedIndirectInternal(pIndirectBuffer, uiOffset, uiDrawCount, uiStride);
}

void spCommandList::DispatchIndirect(spResourceHandle hIndirectBuffer, ezUInt32 uiOffset)
{
  auto pIndirectBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hIndirectBuffer);
  EZ_ASSERT_DEV(pIndirectBuffer != nullptr, "Invalid buffer handle.");

  EZ_ASSERT_DEV(pIndirectBuffer->GetUsage().IsSet(spBufferUsage::IndirectBuffer), "The buffer must have been created with the IndirectBuffer usage flag.");
  EZ_ASSERT_DEV(uiOffset % 4 == 0, "Offset must be aligned to 4 bytes");

  DispatchIndirectInternal(pIndirectBuffer, uiOffset);
}

void spCommandList::ResolveTexture(spResourceHandle hSource, spResourceHandle hDestination)
{
  auto pSource = m_pDevice->GetResourceManager()->GetResource<spTexture>(hSource);
  EZ_ASSERT_DEV(pSource != nullptr, "Invalid texture handle.");

  auto pDestination = m_pDevice->GetResourceManager()->GetResource<spTexture>(hDestination);
  EZ_ASSERT_DEV(pDestination != nullptr, "Invalid texture handle.");

  EZ_ASSERT_DEV(pSource->GetSampleCount() != spTextureSampleCount::None, "Source texture must be a multisample texture.");
  EZ_ASSERT_DEV(pDestination->GetSampleCount() == spTextureSampleCount::None, "Destination texture must be a non-multisample texture.");

  ResolveTextureInternal(pSource, pDestination);
}

void spCommandList::SetFramebuffer(spResourceHandle hFramebuffer)
{
  auto pFramebuffer = m_pDevice->GetResourceManager()->GetResource<spFramebuffer>(hFramebuffer);

  if (m_pFramebuffer != pFramebuffer)
  {
    m_pFramebuffer = pFramebuffer;
    SetFramebufferInternal(pFramebuffer);
    SetFullViewport();
    SetFullScissorRect();
  }
}

void spCommandList::SetIndexBuffer(spResourceHandle hIndexBuffer, const ezEnum<spIndexFormat>& eFormat)
{
  SetIndexBuffer(hIndexBuffer, eFormat, 0);
}

void spCommandList::SetIndexBuffer(spResourceHandle hIndexBuffer, const ezEnum<spIndexFormat>& eFormat, ezUInt32 uiOffset)
{
  auto pIndexBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hIndexBuffer);
  EZ_ASSERT_DEV(pIndexBuffer->GetUsage().IsSet(spBufferUsage::IndexBuffer), "Buffer is not an index buffer.");

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  m_pIndexBuffer = pIndexBuffer;
  m_IndexBufferFormat = eFormat;
#endif

  SetIndexBufferInternal(pIndexBuffer, eFormat, uiOffset);
}

void spCommandList::SetComputeResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet)
{
  SetComputeResourceSet(uiSlot, hResourceSet, 0, nullptr);
}

void spCommandList::SetComputeResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezArrayPtr<ezUInt32> dynamicOffsets)
{
  SetComputeResourceSet(uiSlot, hResourceSet, dynamicOffsets.GetCount(), dynamicOffsets.GetPtr());
}

void spCommandList::SetComputeResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
{
  auto pResourceSet = m_pDevice->GetResourceManager()->GetResource<spResourceSet>(hResourceSet);
  EZ_ASSERT_DEV(pResourceSet != nullptr, "Invalid resource set handle.");

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  EZ_ASSERT_DEV(m_pComputePipeline != nullptr, "Compute pipeline has not been set.");

  ezUInt32 uiLayoutCount = m_pGraphicPipeline->GetResourceLayouts().GetCount();
  EZ_ASSERT_DEV(uiSlot < uiLayoutCount, "Slot out of range. The active pipeline has {0} slots.", uiLayoutCount);

  const auto pPipelineLayout = m_pGraphicPipeline->GetResourceLayout(uiSlot);
  EZ_ASSERT_DEV(pPipelineLayout != nullptr, "Slot {0} does not have a valid layout.", uiSlot);

  const auto pResourceLayout = m_pDevice->GetResourceManager()->GetResource<spResourceLayout>(pResourceSet->GetLayout());
  EZ_ASSERT_DEV(pResourceLayout != nullptr, "Invalid resource layout handle from the resource set.");

  ezUInt32 uiPipelineLayoutElementsCount = pPipelineLayout->GetElementCount();
  const auto& layoutElements = pResourceLayout->GetElements();
  EZ_ASSERT_DEV(uiPipelineLayoutElementsCount == layoutElements.GetCount(), "Slot {0} does not have the same number of elements ({1}) as the pipeline layout ({2}).", uiSlot, layoutElements.GetCount(), uiPipelineLayoutElementsCount);

  for (ezUInt32 i = 0; i < uiPipelineLayoutElementsCount; ++i)
  {
    ezEnum<spShaderResourceType> ePipelineResourceType = pPipelineLayout->GetDescription().m_Elements[i].m_eType;
    ezEnum<spShaderResourceType> eSetResourceType = layoutElements[i].m_eType;
    EZ_ASSERT_DEV(ePipelineResourceType == eSetResourceType, "Slot {0} does not have the same element type ({1}) as the pipeline layout ({2}).", uiSlot, eSetResourceType, ePipelineResourceType);
  }
#endif

  SetComputeResourceSetInternal(uiSlot, pResourceSet, uiDynamicOffsetCount, pDynamicOffsets);
}

void spCommandList::SetGraphicResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet)
{
  SetGraphicResourceSet(uiSlot, hResourceSet, 0, nullptr);
}

void spCommandList::SetGraphicResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezArrayPtr<ezUInt32> dynamicOffsets)
{
  SetGraphicResourceSet(uiSlot, hResourceSet, dynamicOffsets.GetCount(), dynamicOffsets.GetPtr());
}

void spCommandList::SetGraphicResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
{
  auto pResourceSet = m_pDevice->GetResourceManager()->GetResource<spResourceSet>(hResourceSet);
  EZ_ASSERT_DEV(pResourceSet != nullptr, "Invalid resource set handle.");

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  EZ_ASSERT_DEV(m_pGraphicPipeline != nullptr, "Graphics pipeline has not been set.");

  ezUInt32 uiLayoutCount = m_pGraphicPipeline->GetResourceLayouts().GetCount();
  EZ_ASSERT_DEV(uiSlot < uiLayoutCount, "Slot out of range. The active pipeline has {0} slots.", uiLayoutCount);

  const auto pPipelineLayout = m_pGraphicPipeline->GetResourceLayout(uiSlot);
  EZ_ASSERT_DEV(pPipelineLayout != nullptr, "Slot {0} does not have a valid layout.", uiSlot);

  const auto pResourceLayout = m_pDevice->GetResourceManager()->GetResource<spResourceLayout>(pResourceSet->GetLayout());
  EZ_ASSERT_DEV(pResourceLayout != nullptr, "Invalid resource layout handle from the resource set.");

  ezUInt32 uiPipelineLayoutElementsCount = pPipelineLayout->GetElementCount();
  const auto& layoutElements = pResourceLayout->GetElements();
  EZ_ASSERT_DEV(uiPipelineLayoutElementsCount == layoutElements.GetCount(), "Slot {0} does not have the same number of elements ({1}) as the pipeline layout ({2}).", uiSlot, layoutElements.GetCount(), uiPipelineLayoutElementsCount);

  for (ezUInt32 i = 0; i < uiPipelineLayoutElementsCount; ++i)
  {
    ezEnum<spShaderResourceType> ePipelineResourceType = pPipelineLayout->GetDescription().m_Elements[i].m_eType;
    ezEnum<spShaderResourceType> eSetResourceType = layoutElements[i].m_eType;
    EZ_ASSERT_DEV(ePipelineResourceType == eSetResourceType, "Slot {0} does not have the same element type ({1}) as the pipeline layout ({2}).", uiSlot, eSetResourceType, ePipelineResourceType);
  }

  EZ_ASSERT_DEV(pResourceLayout->GetDynamicBufferCount() == uiDynamicOffsetCount, "A dynamic offset must be provided for each resource that specifies the DynamicBinding option. {0} offsets were found, but {1} were expected.", uiDynamicOffsetCount, pResourceLayout->GetDynamicBufferCount());

  ezUInt32 uiDynamicOffset = 0;
  const auto& pOffsets = ezMakeArrayPtr(pDynamicOffsets, uiDynamicOffsetCount);

  for (ezUInt32 i = 0, l = layoutElements.GetCount(); i < l; ++i)
  {
    if (layoutElements[i].m_eOptions.IsSet(spResourceLayoutElementOptions::DynamicBinding))
    {
      ezUInt32 uiAlignment = layoutElements[i].m_eType == spShaderResourceType::ConstantBuffer ? m_pDevice->GetConstantBufferMinOffsetAlignment() : m_pDevice->GetStructuredBufferMinOffsetAlignment();
      ezUInt32 uiOffset = pOffsets[uiDynamicOffset];
      uiDynamicOffset++;

      spBufferRange* pBufferRange = spResourceHelper::GetBufferRange(m_pDevice, pResourceSet->GetResource(i), uiOffset);
      EZ_ASSERT_DEV(pBufferRange->GetOffset() % uiAlignment == 0, "Offset must be aligned to {0} bytes.", uiAlignment);
    }
  }
#endif

  SetGraphicResourceSetInternal(uiSlot, pResourceSet, uiDynamicOffsetCount, pDynamicOffsets);
}

void spCommandList::SetVertexBuffer(ezUInt32 uiSlot, spResourceHandle hVertexBuffer)
{
  SetVertexBuffer(uiSlot, hVertexBuffer, 0);
}

void spCommandList::SetVertexBuffer(ezUInt32 uiSlot, spResourceHandle hVertexBuffer, ezUInt32 uiOffset)
{
  auto pVertexBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hVertexBuffer);
  EZ_ASSERT_DEV(pVertexBuffer->GetUsage().IsSet(spBufferUsage::VertexBuffer), "Buffer is not a vertex buffer.");

  SetVertexBufferInternal(uiSlot, pVertexBuffer, uiOffset);
}

void spCommandList::SetFullScissorRect()
{
  SetFullScissorRect(0);

  for (ezUInt32 i = 1, l = m_pFramebuffer->GetColorTargets().GetCount(); i < l; ++i)
    SetFullScissorRect(i);
}

void spCommandList::SetFullScissorRect(ezUInt32 uiSlot)
{
  SetScissorRect(uiSlot, 0, 0, m_pFramebuffer->GetWidth(), m_pFramebuffer->GetHeight());
}

void spCommandList::SetFullViewport()
{
  SetFullViewport(0);

  for (ezUInt32 i = 1, l = m_pFramebuffer->GetColorTargets().GetCount(); i < l; ++i)
    SetFullViewport(i);
}

void spCommandList::SetFullViewport(ezUInt32 uiSlot)
{
  SetViewport(uiSlot, spViewport(0, 0, m_pFramebuffer->GetWidth(), m_pFramebuffer->GetHeight(), 0, 1));
}

void spCommandList::UpdateBuffer(spResourceHandle hBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize)
{
  auto pBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hBuffer);
  EZ_ASSERT_DEV(pBuffer != nullptr, "Invalid buffer handle.");

  EZ_ASSERT_DEV(uiOffset + uiSize <= pBuffer->GetSize(), "The buffer is not large enough to contain the new data.");

  if (uiSize == 0)
    return;

  UpdateBufferInternal(pBuffer, uiOffset, pSourceData, uiSize);
}

void spCommandList::CopyBuffer(spResourceHandle hSourceBuffer, ezUInt32 uiSourceOffset, spResourceHandle hDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize)
{
  auto pSourceBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hSourceBuffer);
  EZ_ASSERT_DEV(pSourceBuffer != nullptr, "Invalid source buffer handle.");

  auto pDestBuffer = m_pDevice->GetResourceManager()->GetResource<spBuffer>(hDestBuffer);
  EZ_ASSERT_DEV(pDestBuffer != nullptr, "Invalid destination buffer handle.");

  if (uiSize == 0)
    return;

  CopyBufferInternal(pSourceBuffer, uiSourceOffset, pDestBuffer, uiDestOffset, uiSize);
}

void spCommandList::CopyTexture(spResourceHandle hSourceTexture, spResourceHandle hDestinationTexture)
{
  auto pSourceTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(hSourceTexture);
  EZ_ASSERT_DEV(pSourceTexture != nullptr, "Invalid source texture handle.");

  auto pDestinationTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(hDestinationTexture);
  EZ_ASSERT_DEV(pDestinationTexture != nullptr, "Invalid destination texture handle.");

  const ezUInt32 uiEffectiveSourceArrayLayers = (pSourceTexture->GetUsage().IsSet(spTextureUsage::Cubemap)) ? pSourceTexture->GetArrayLayerCount() * 6 : pSourceTexture->GetArrayLayerCount();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  const ezUInt32 uiEffectiveDestArrayLayers = (pDestinationTexture->GetUsage().IsSet(spTextureUsage::Cubemap)) ? pDestinationTexture->GetArrayLayerCount() * 6 : pDestinationTexture->GetArrayLayerCount();
  EZ_ASSERT_DEV(uiEffectiveSourceArrayLayers == uiEffectiveDestArrayLayers && pSourceTexture->GetMipCount() == pDestinationTexture->GetMipCount() && pSourceTexture->GetSampleCount() == pDestinationTexture->GetSampleCount() && pSourceTexture->GetWidth() == pDestinationTexture->GetWidth() && pSourceTexture->GetHeight() == pDestinationTexture->GetHeight() && pSourceTexture->GetDepth() == pDestinationTexture->GetDepth() && pSourceTexture->GetFormat() == pDestinationTexture->GetFormat(), "Source and destination textures are not compatible for copy.");
#endif

  for (ezUInt32 uiLevel = 0, l = pSourceTexture->GetMipCount(); uiLevel < l; uiLevel++)
  {
    ezUInt32 uiWidth = 0, uiHeight = 0, uiDepth = 0;
    spTextureHelper::GetMipDimensions(pSourceTexture, uiLevel, uiWidth, uiHeight, uiDepth);

    CopyTextureInternal(pSourceTexture, 0, 0, 0, uiLevel, 0, pDestinationTexture, 0, 0, 0, uiLevel, 0, uiWidth, uiHeight, uiDepth, uiEffectiveSourceArrayLayers);
  }
}

void spCommandList::CopyTexture(spResourceHandle hSourceTexture, spResourceHandle hDestinationTexture, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer)
{
  auto pSourceTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(hSourceTexture);
  EZ_ASSERT_DEV(pSourceTexture != nullptr, "Invalid source texture handle.");

  auto pDestinationTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(hDestinationTexture);
  EZ_ASSERT_DEV(pDestinationTexture != nullptr, "Invalid destination texture handle.");

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  const ezUInt32 uiEffectiveSourceArrayLayers = (pSourceTexture->GetUsage().IsSet(spTextureUsage::Cubemap)) ? pSourceTexture->GetArrayLayerCount() * 6 : pSourceTexture->GetArrayLayerCount();
  const ezUInt32 uiEffectiveDestArrayLayers = (pDestinationTexture->GetUsage().IsSet(spTextureUsage::Cubemap)) ? pDestinationTexture->GetArrayLayerCount() * 6 : pDestinationTexture->GetArrayLayerCount();

  EZ_ASSERT_DEV(pSourceTexture->GetSampleCount() == pDestinationTexture->GetSampleCount() && pSourceTexture->GetWidth() == pDestinationTexture->GetWidth() && pSourceTexture->GetHeight() == pDestinationTexture->GetHeight() && pSourceTexture->GetDepth() == pDestinationTexture->GetDepth() && pSourceTexture->GetFormat() == pDestinationTexture->GetFormat(), "Source and destination textures are not compatible for copy.");
  EZ_ASSERT_DEV(uiMipLevel < pSourceTexture->GetMipCount() && uiMipLevel < pDestinationTexture->GetMipCount(), "Invalid mip level.");
  EZ_ASSERT_DEV(uiArrayLayer < uiEffectiveSourceArrayLayers && uiArrayLayer < uiEffectiveDestArrayLayers, "Invalid array layer.");
#endif

  ezUInt32 uiWidth = 0, uiHeight = 0, uiDepth = 0;
  spTextureHelper::GetMipDimensions(pSourceTexture, uiMipLevel, uiWidth, uiHeight, uiDepth);
  CopyTextureInternal(pSourceTexture, 0, 0, 0, uiMipLevel, uiArrayLayer, pDestinationTexture, 0, 0, 0, uiMipLevel, uiArrayLayer, uiWidth, uiHeight, uiDepth, 1);
}

void spCommandList::CopyTexture(spResourceHandle hSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, spResourceHandle hDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount)
{
  auto pSourceTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(hSourceTexture);
  EZ_ASSERT_DEV(pSourceTexture != nullptr, "Invalid source texture handle.");

  auto pDestinationTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(hDestinationTexture);
  EZ_ASSERT_DEV(pDestinationTexture != nullptr, "Invalid destination texture handle.");

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  EZ_ASSERT_DEV(uiWidth != 0 && uiHeight != 0 && uiDepth != 0, "The given region is empty.");
  EZ_ASSERT_DEV(uiLayerCount != 0, "The given layer count is 0.");

  ezUInt32 uiSrcWidth = 0, uiSrcHeight = 0, uiSrcDepth = 0;
  spTextureHelper::GetMipDimensions(pSourceTexture, uiSourceMipLevel, uiSrcWidth, uiSrcHeight, uiSrcDepth);
  const ezUInt32 uiSrcBlockSize = spPixelFormatHelper::IsCompressedFormat(pSourceTexture->GetFormat()) ? 4u : 1u;
  const ezUInt32 uiRoundedSrcWidth = (uiSrcWidth + uiSrcBlockSize - 1) / (uiSrcBlockSize * uiSrcBlockSize);
  const ezUInt32 uiRoundedSrcHeight = (uiSrcHeight + uiSrcBlockSize - 1) / (uiSrcBlockSize * uiSrcBlockSize);
  EZ_ASSERT_DEV(uiSourceX + uiWidth <= uiRoundedSrcWidth && uiSourceY + uiHeight <= uiRoundedSrcHeight, "Source region is too large.");

  ezUInt32 uiDstWidth = 0, uiDstHeight = 0, uiDstDepth = 0;
  spTextureHelper::GetMipDimensions(pDestinationTexture, uiDestMipLevel, uiDstWidth, uiDstHeight, uiDstDepth);
  const ezUInt32 uiDstBlockSize = spPixelFormatHelper::IsCompressedFormat(pDestinationTexture->GetFormat()) ? 4u : 1u;
  const ezUInt32 uiRoundedDstWidth = (uiDstWidth + uiDstBlockSize - 1) / (uiDstBlockSize * uiDstBlockSize);
  const ezUInt32 uiRoundedDstHeight = (uiDstHeight + uiDstBlockSize - 1) / (uiDstBlockSize * uiDstBlockSize);
  EZ_ASSERT_DEV(uiDestX + uiWidth <= uiRoundedDstWidth && uiDestY + uiHeight <= uiRoundedDstHeight, "Destination region is too large.");

  EZ_ASSERT_DEV(uiSourceMipLevel < pSourceTexture->GetMipCount(), "Source mip level is out of bounds.");
  EZ_ASSERT_DEV(uiDestMipLevel < pDestinationTexture->GetMipCount(), "Destination mip level is out of bounds.");

  const ezUInt32 uiSrcLayers = pSourceTexture->GetUsage().IsSet(spTextureUsage::Cubemap) ? pSourceTexture->GetArrayLayerCount() * 6 : pSourceTexture->GetArrayLayerCount();
  EZ_ASSERT_DEV(uiSourceBaseArrayLayer + uiLayerCount <= uiSrcLayers, "Source array layer is out of bounds.");

  const ezUInt32 uiDestLayers = pDestinationTexture->GetUsage().IsSet(spTextureUsage::Cubemap) ? pDestinationTexture->GetArrayLayerCount() * 6 : pDestinationTexture->GetArrayLayerCount();
  EZ_ASSERT_DEV(uiDestBaseArrayLayer + uiLayerCount <= uiDestLayers, "Destination array layer is out of bounds.");
#endif

  CopyTextureInternal(pSourceTexture, uiSourceX, uiSourceY, uiSourceZ, uiSourceMipLevel, uiSourceBaseArrayLayer, pDestinationTexture, uiDestX, uiDestY, uiDestZ, uiDestMipLevel, uiDestBaseArrayLayer, uiWidth, uiHeight, uiDepth, uiLayerCount);
}

void spCommandList::GenerateMipmaps(spResourceHandle hTexture)
{
  auto pTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(hTexture);
  EZ_ASSERT_DEV(pTexture != nullptr, "Invalid texture handle.");

  EZ_ASSERT_DEV(pTexture->GetUsage().IsSet(spTextureUsage::GenerateMipmaps), "The texture was not created with the GenerateMipmaps texture usage flag.");

  if (pTexture->GetMipCount() <= 1)
    return;

  GenerateMipmapsInternal(pTexture);
}

void spCommandList::ClearCachedState()
{
  m_pFramebuffer = nullptr;
  m_pGraphicPipeline = nullptr;
  m_pComputePipeline = nullptr;
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  m_pIndexBuffer = nullptr;
#endif
}

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
void spCommandList::IndexBufferValidation(ezUInt32 uiIndexCount) const
{
  EZ_ASSERT_DEV(m_pIndexBuffer != nullptr, "An index buffer must be set before calling DrawIndexed.");

  const ezUInt32 uiIndexFormatSize = m_IndexBufferFormat == spIndexFormat::UInt16 ? sizeof(ezUInt16) : sizeof(ezUInt32);
  ezUInt32 uiBytesNeeded = uiIndexCount * uiIndexFormatSize;
  EZ_ASSERT_DEV(m_pIndexBuffer->GetSize() >= uiBytesNeeded, "The active index buffer is too small for the draw command. {0} bytes needed, but only {1} available.", uiBytesNeeded, m_pIndexBuffer->GetSize());
}

void spCommandList::DrawValidation() const
{
  EZ_ASSERT_DEV(m_pGraphicPipeline != nullptr, "A graphic pipeline must be set before calling a Draw command.");
  EZ_ASSERT_DEV(m_pFramebuffer != nullptr, "A framebuffer must be set before calling a Draw command.");
  EZ_ASSERT_DEV(m_pGraphicPipeline->GetOutputDescription() == m_pFramebuffer->GetOutputDescription(), "The output description of the graphic pipeline must match the output description of the framebuffer.");
}
#endif

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_CommandList);
