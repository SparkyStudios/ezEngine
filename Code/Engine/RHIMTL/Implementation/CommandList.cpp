#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Buffer.h>
#include <RHIMTL/CommandList.h>
#include <RHIMTL/Core.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Framebuffer.h>
#include <RHIMTL/Pipeline.h>
#include <RHIMTL/ResourceLayout.h>
#include <RHIMTL/ResourceSet.h>
#include <RHIMTL/Sampler.h>
#include <RHIMTL/Shader.h>
#include <RHIMTL/Swapchain.h>
#include <RHIMTL/Texture.h>

// clang-format off
EZ_DEFINE_AS_POD_TYPE(MTL::Viewport);
EZ_DEFINE_AS_POD_TYPE(MTL::ScissorRect);
// clang-format on

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spCommandListMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spCommandListMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    End();
    Reset();

    {
      EZ_LOCK(m_SubmittedCommandsLock);

      for (auto& buffer : m_AvailableStagingBuffers)
        buffer.Clear();

      m_AvailableStagingBuffers.Clear();

      for (auto& pair : m_SubmittedStagingBuffers)
      {
        for (auto& buffer : pair.Value())
          buffer.Clear();

        pair.Value().Clear();
      }

      m_SubmittedStagingBuffers.Clear();
    }

    SP_RHI_MTL_RELEASE(m_pCommandBuffer);

    m_bReleased = true;
  }

  bool spCommandListMTL::IsReleased() const
  {
    return m_bReleased;
  }

  void spCommandListMTL::Begin()
  {
    SP_RHI_MTL_RELEASE(m_pCommandBuffer);

    const auto* pDevice = static_cast<spDeviceMTL*>(m_pDevice);

    m_pCommandBuffer = pDevice->GetCommandQueue()->commandBuffer();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    {
      spScopedMTLResource nsString(NS::String::string("RHI Metal Command List Buffer", NS::UTF8StringEncoding));
      m_pCommandBuffer->setLabel(*nsString);
    }
#endif

    ClearCachedState();
  }

  void spCommandListMTL::Dispatch(ezUInt32 uiGroupCountX, ezUInt32 uiGroupCountY, ezUInt32 uiGroupCountZ)
  {
    PreDispatch();
    m_pComputeCommandEncoder->dispatchThreadgroups(MTL::Size(uiGroupCountX, uiGroupCountY, uiGroupCountZ), m_pComputePipeline.Downcast<spComputePipelineMTL>()->GetDispatchSize());
  }

  void spCommandListMTL::SetComputePipeline(ezSharedPtr<spComputePipeline> pComputePipeline)
  {
    EZ_ASSERT_DEV(pComputePipeline != nullptr, "Invalid compute pipeline handle");

    if (m_pComputePipeline == pComputePipeline)
      return;

    const ezUInt32 uiLayoutsCount = pComputePipeline->GetResourceLayouts().GetCount();

    m_ComputeResourceSets.EnsureCount(uiLayoutsCount);
    m_ActiveComputeResourceSets.EnsureCount(uiLayoutsCount);

    ezMemoryUtils::ZeroFill(m_ActiveComputeResourceSets.GetData(), uiLayoutsCount);

    m_pComputePipeline = pComputePipeline;
    m_bComputePipelineChanged = true;
  }

  void spCommandListMTL::SetGraphicPipeline(ezSharedPtr<spGraphicPipeline> pGraphicPipeline)
  {
    EZ_ASSERT_DEV(pGraphicPipeline != nullptr, "Invalid graphics pipeline handle");

    if (m_pGraphicPipeline == pGraphicPipeline)
      return;

    const ezUInt32 uiLayoutsCount = pGraphicPipeline->GetResourceLayouts().GetCount();

    m_GraphicResourceSets.EnsureCount(uiLayoutsCount);
    m_ActiveGraphicResourceSets.EnsureCount(uiLayoutsCount);

    ezMemoryUtils::ZeroFill(m_ActiveGraphicResourceSets.GetData(), uiLayoutsCount);

    m_uiNumVertexBuffers = pGraphicPipeline.Downcast<spGraphicPipelineMTL>()->GetVertexBufferCount();

    m_VertexBuffers.EnsureCount(m_uiNumVertexBuffers);
    m_VertexOffsets.EnsureCount(m_uiNumVertexBuffers);
    m_ActiveVertexBuffers.EnsureCount(m_uiNumVertexBuffers);

    ezMemoryUtils::ZeroFill(m_ActiveVertexBuffers.GetData(), m_uiNumVertexBuffers);

    m_PushConstant.m_pData = nullptr;

    m_pGraphicPipeline = pGraphicPipeline;
    m_bGraphicPipelineChanged = true;
  }

  void spCommandListMTL::SetScissorRect(ezUInt32 uiSlot, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiWidth, ezUInt32 uiHeight)
  {
    EZ_ASSERT_DEV(uiSlot < m_ScissorRects.GetCount(), "Invalid scissor rect slot");

    m_bScissorRectsChanged = true;

    m_ScissorRects[uiSlot].x = uiX;
    m_ScissorRects[uiSlot].y = uiY;
    m_ScissorRects[uiSlot].width = uiWidth;
    m_ScissorRects[uiSlot].height = uiHeight;
  }

  void spCommandListMTL::SetViewport(ezUInt32 uiSlot, const spViewport& viewport)
  {
    EZ_ASSERT_DEV(uiSlot < m_Viewports.GetCount(), "Invalid viewport slot");

    m_bViewportsChanged = true;

    m_Viewports[uiSlot].originX = viewport.m_iX;
    m_Viewports[uiSlot].originY = viewport.m_iY;
    m_Viewports[uiSlot].width = viewport.m_uiWidth;
    m_Viewports[uiSlot].height = viewport.m_uiHeight;
    m_Viewports[uiSlot].znear = viewport.m_fMinDepth;
    m_Viewports[uiSlot].zfar = viewport.m_fMaxDepth;
  }

  void spCommandListMTL::PushProfileScope(ezStringView sName)
  {
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    // TODO: Use Metal profiling system
#endif

    PushDebugGroup(sName);
  }

  void spCommandListMTL::PopProfileScope(ezSharedPtr<spScopeProfiler>& scopeProfiler)
  {
    PopDebugGroup();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    // TODO: Use Metal profiling system
#endif
  }

  void spCommandListMTL::PushDebugGroup(ezStringView sName)
  {
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    spScopedMTLResource nsName(NS::String::string(sName.GetData(sb), NS::UTF8StringEncoding));

    if (m_pBlitCommandEncoder != nullptr)
      m_pBlitCommandEncoder->pushDebugGroup(*nsName);
    else if (m_pComputeCommandEncoder != nullptr)
      m_pComputeCommandEncoder->pushDebugGroup(*nsName);
    else if (m_pRenderCommandEncoder != nullptr)
      m_pRenderCommandEncoder->pushDebugGroup(*nsName);

    m_bIsInDebugGroup = true;
    m_sDebugGroupName = sName;
#endif
  }

  void spCommandListMTL::PopDebugGroup()
  {
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    if (m_pBlitCommandEncoder != nullptr)
      m_pBlitCommandEncoder->popDebugGroup();
    else if (m_pComputeCommandEncoder != nullptr)
      m_pComputeCommandEncoder->popDebugGroup();
    else if (m_pRenderCommandEncoder != nullptr)
      m_pRenderCommandEncoder->popDebugGroup();

    m_bIsInDebugGroup = false;
    m_sDebugGroupName = "";
#endif
  }

  void spCommandListMTL::InsertDebugMarker(ezStringView sName)
  {
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    spScopedMTLResource nsName(NS::String::string(sName.GetData(sb), NS::UTF8StringEncoding));

    if (m_pBlitCommandEncoder != nullptr)
      m_pBlitCommandEncoder->insertDebugSignpost(*nsName);
    else if (m_pComputeCommandEncoder != nullptr)
      m_pComputeCommandEncoder->insertDebugSignpost(*nsName);
    else if (m_pRenderCommandEncoder != nullptr)
      m_pRenderCommandEncoder->insertDebugSignpost(*nsName);
#endif
  }

  void spCommandListMTL::End()
  {
    EnsureNoBlitEncoder();
    EnsureNoComputeEncoder();

    if (!m_bCurrentFramebufferEverActive && m_pFramebuffer != nullptr && HasAnyUnsetClearValues())
      BeginCurrentRenderPass();

    EnsureNoRenderPass();
  }

  void spCommandListMTL::Reset()
  {
    ClearCachedState();
    ClearBoundResources();
  }

  void spCommandListMTL::ClearColorTargetInternal(ezUInt32 uiIndex, ezColor clearColor)
  {
    EZ_ASSERT_DEV(uiIndex < m_ClearColors.GetCount(), "Invalid render target index");

    EnsureNoRenderPass();
    m_ClearColors[uiIndex] = clearColor;
  }

  void spCommandListMTL::ClearDepthStencilTargetInternal(float fClearDepth, ezUInt8 uiClearStencil)
  {
    EnsureNoRenderPass();
    m_fClearDepth = fClearDepth;
    m_uiClearStencil = uiClearStencil;
  }

  void spCommandListMTL::DrawInternal(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart)
  {
    if (!PreDraw())
      return;

    if (uiInstanceStart == 0)
      m_pRenderCommandEncoder->drawPrimitives(spToMTL(m_pGraphicPipeline->GetPrimitiveTopology()), uiVertexStart, uiVertexCount, uiInstanceCount);
    else
      m_pRenderCommandEncoder->drawPrimitives(spToMTL(m_pGraphicPipeline->GetPrimitiveTopology()), uiVertexStart, uiVertexCount, uiInstanceCount, uiInstanceStart);
  }

  void spCommandListMTL::DrawIndexedInternal(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart)
  {
    if (!PreDraw())
      return;

    const ezUInt32 uiIndexSize = m_eIndexType == MTL::IndexTypeUInt16 ? 2u : 4u;
    const ezUInt32 uiIndexBufferOffset = (uiIndexSize * uiIndexStart) + m_uiIndexBufferOffset;

    if (uiInstanceStart == 0)
    {
      m_pRenderCommandEncoder->drawIndexedPrimitives(
        spToMTL(m_pGraphicPipeline->GetPrimitiveTopology()),
        uiIndexCount,
        m_eIndexType,
        m_pIndexBuffer->GetMTLBuffer(),
        uiIndexBufferOffset,
        uiInstanceCount);
    }
    else
    {
      m_pRenderCommandEncoder->drawIndexedPrimitives(
        spToMTL(m_pGraphicPipeline->GetPrimitiveTopology()),
        uiIndexCount,
        m_eIndexType,
        m_pIndexBuffer->GetMTLBuffer(),
        uiVertexOffset,
        uiInstanceCount,
        uiIndexBufferOffset,
        uiInstanceStart);
    }
  }

  void spCommandListMTL::DrawIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
  {
    if (!PreDraw())
      return;

    const auto pIndirectBufferMTL = pIndirectBuffer.Downcast<spBufferMTL>();

    for (ezUInt32 uiDraw = 0; uiDraw < uiDrawCount; ++uiDraw)
    {
      const ezUInt32 uiCurrentOffset = uiDraw * uiStride + uiOffset;
      m_pRenderCommandEncoder->drawPrimitives(
        spToMTL(m_pGraphicPipeline->GetPrimitiveTopology()),
        pIndirectBufferMTL->GetMTLBuffer(),
        uiCurrentOffset);
    }
  }

  void spCommandListMTL::DrawIndexedIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride)
  {
    if (!PreDraw())
      return;

    const auto pIndirectBufferMTL = pIndirectBuffer.Downcast<spBufferMTL>();

    for (ezUInt32 uiDraw = 0; uiDraw < uiDrawCount; ++uiDraw)
    {
      const ezUInt32 uiCurrentOffset = uiDraw * uiStride + uiOffset;
      m_pRenderCommandEncoder->drawIndexedPrimitives(
        spToMTL(m_pGraphicPipeline->GetPrimitiveTopology()),
        m_eIndexType,
        m_pIndexBuffer->GetMTLBuffer(),
        m_uiIndexBufferOffset,
        pIndirectBufferMTL->GetMTLBuffer(),
        uiCurrentOffset);
    }
  }

  void spCommandListMTL::DispatchIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset)
  {
    PreDispatch();

    const auto pIndirectBufferMTL = pIndirectBuffer.Downcast<spBufferMTL>();
    m_pComputeCommandEncoder->dispatchThreadgroups(
      pIndirectBufferMTL->GetMTLBuffer(),
      uiOffset,
      m_pComputePipeline.Downcast<spComputePipelineMTL>()->GetDispatchSize());
  }

  void spCommandListMTL::ResolveTextureInternal(ezSharedPtr<spTexture> pSource, ezSharedPtr<spTexture> pDestination)
  {
    // TODO: This approach destroys the contents of the source Texture (according to the docs).
    EnsureNoBlitEncoder();
    EnsureNoRenderPass();

    const auto pSourceMTL = pSource.Downcast<spTextureMTL>();
    const auto pDestinationMTL = pDestination.Downcast<spTextureMTL>();

    spScopedMTLResource rpDesc(MTL::RenderPassDescriptor::alloc()->init());
    const auto colorAttachment = rpDesc->colorAttachments()->object(0);
    colorAttachment->setTexture(pSourceMTL->GetMTLTexture());
    colorAttachment->setLoadAction(MTL::LoadActionLoad);
    colorAttachment->setStoreAction(MTL::StoreActionMultisampleResolve);
    colorAttachment->setResolveTexture(pDestinationMTL->GetMTLTexture());

    {
      spScopedMTLResource autoReleasePool(NS::AutoreleasePool::alloc()->init());
      MTL::RenderCommandEncoder* encoder = m_pCommandBuffer->renderCommandEncoder(*rpDesc);
      encoder->endEncoding();
    }
  }

  void spCommandListMTL::SetFramebufferInternal(ezSharedPtr<spFramebuffer> pFramebuffer)
  {
    if (!m_bCurrentFramebufferEverActive && m_pFramebuffer != nullptr)
    {
      // This ensures that any submitted clear values will be used even if nothing has been drawn.
      if (HasAnyUnsetClearValues() && EnsureRenderPass())
        EndCurrentRenderPass();
    }

    EnsureNoRenderPass();

    m_pFramebuffer = pFramebuffer.Downcast<spFramebufferMTLBase>();
    m_pFramebuffer->EnsureResourceCreated();

    const ezUInt32 uiColorTargetCount = pFramebuffer->GetColorTargetCount();
    const ezUInt32 uiViewportCount = ezMath::Max(1u, uiColorTargetCount);

    m_Viewports.EnsureCount(uiViewportCount);
    m_ScissorRects.EnsureCount(uiViewportCount);
    m_ClearColors.EnsureCount(uiColorTargetCount);

    m_bCurrentFramebufferEverActive = false;
  }

  void spCommandListMTL::SetIndexBufferInternal(ezSharedPtr<spBuffer> pIndexBuffer, ezEnum<spIndexFormat> eFormat, ezUInt32 uiOffset)
  {
    m_pIndexBuffer = pIndexBuffer.Downcast<spBufferMTL>();
    m_uiIndexBufferOffset = uiOffset;
    m_eIndexType = spToMTL(eFormat);
  }

  void spCommandListMTL::SetComputeResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
  {
    spCommandListResourceSet& set = m_ComputeResourceSets[uiSlot];

    if (IsResourceSetEqual(set, pResourceSet, uiDynamicOffsetCount, pDynamicOffsets))
      return;

    set = spCommandListResourceSet(pResourceSet->GetHandle(), uiDynamicOffsetCount, pDynamicOffsets);
    m_ActiveComputeResourceSets[uiSlot] = false;

    ClearBoundResources();
  }

  void spCommandListMTL::SetGraphicResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
  {
    spCommandListResourceSet& set = m_GraphicResourceSets[uiSlot];

    if (IsResourceSetEqual(set, pResourceSet, uiDynamicOffsetCount, pDynamicOffsets))
      return;

    set = spCommandListResourceSet(pResourceSet->GetHandle(), uiDynamicOffsetCount, pDynamicOffsets);
    m_ActiveGraphicResourceSets[uiSlot] = false;

    ClearBoundResources();
  }

  void spCommandListMTL::SetVertexBufferInternal(ezUInt32 uiSlot, ezSharedPtr<spBuffer> pVertexBuffer, ezUInt32 uiOffset)
  {
    const auto pBuffer = pVertexBuffer.Downcast<spBufferMTL>();
    pBuffer->EnsureResourceCreated();

    m_VertexBuffers.EnsureCount(uiSlot + 1);
    m_VertexOffsets.EnsureCount(uiSlot + 1);
    m_ActiveVertexBuffers.EnsureCount(uiSlot + 1);

    if (m_VertexBuffers[uiSlot] != pBuffer || m_VertexOffsets[uiSlot] != uiOffset)
    {
      m_VertexBuffers[uiSlot] = pBuffer;
      m_VertexOffsets[uiSlot] = uiOffset;
      m_ActiveVertexBuffers[uiSlot] = false;

      m_uiNumVertexBuffers = ezMath::Max(m_uiNumVertexBuffers, uiSlot + 1);
    }
  }

  void spCommandListMTL::PushConstantsInternal(ezBitflags<spShaderStage> eStage, const void* pData, ezUInt32 uiOffset, ezUInt32 uiSize)
  {
    EZ_ASSERT_DEV(m_pGraphicPipeline != nullptr || m_pComputePipeline != nullptr, "Push constants can only be set after a pipeline.");
    EZ_ASSERT_DEV(m_pGraphicPipeline == nullptr || m_pGraphicPipeline->SupportsPushConstants(), "Push constants are not supported by this pipeline. Please enable push constants when creating the pipeline.");
    EZ_ASSERT_DEV(m_pComputePipeline == nullptr || m_pComputePipeline->SupportsPushConstants(), "Push constants are not supported by this pipeline. Please enable push constants when creating the pipeline.");

    m_PushConstant.m_eStage = eStage;
    m_PushConstant.m_pData = pData;
    m_PushConstant.m_uiOffset = uiOffset;
    m_PushConstant.m_uiSize = uiSize;
  }

  void spCommandListMTL::UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize)
  {
    const auto pBufferMTL = pBuffer.Downcast<spBufferMTL>();
    pBufferMTL->EnsureResourceCreated();

    const auto pBufferRangeMTL = pBufferMTL->GetCurrentRange();
    uiOffset += pBufferRangeMTL->GetOffset();

    const bool bUseComputeCopy = (uiOffset % 4 != 0) || (uiSize % 4 != 0 && uiOffset != 0 && uiSize != pBuffer->GetSize());

    const auto pStagingBuffer = GetFreeStagingBuffer(uiSize);
    m_pDevice->UpdateBuffer(pStagingBuffer, 0, pSourceData, uiSize);

    if (bUseComputeCopy)
    {
      CopyBufferInternal(pStagingBuffer, 0, pBuffer, uiOffset, uiSize);
    }
    else
    {
      EZ_ASSERT_DEV(uiOffset % 4 == 0, "Buffer offset must be a multiple of 4.");
      const uint sizeRoundFactor = (4 - (uiSize % 4)) % 4;
      EnsureBlitEncoder();

      m_pBlitCommandEncoder->copyFromBuffer(
        pStagingBuffer->GetMTLBuffer(), 0,
        pBufferMTL->GetMTLBuffer(), uiOffset,
        (uiSize + sizeRoundFactor));
    }

    {
      EZ_LOCK(m_SubmittedCommandsLock);

      ezDynamicArray<ezSharedPtr<spBufferMTL>>* stagingBuffers;
      if (!m_SubmittedStagingBuffers.TryGetValue(m_pCommandBuffer, stagingBuffers))
      {
        m_SubmittedStagingBuffers[m_pCommandBuffer] = ezDynamicArray<ezSharedPtr<spBufferMTL>>();
        stagingBuffers = &m_SubmittedStagingBuffers[m_pCommandBuffer];
      }

      stagingBuffers->PushBack(pStagingBuffer);
    }
  }

  void spCommandListMTL::CopyBufferInternal(ezSharedPtr<spBuffer> pSourceBuffer, ezUInt32 uiSourceOffset, ezSharedPtr<spBuffer> pDestinationBuffer, ezUInt32 uiDestinationOffset, ezUInt32 uiSize)
  {
    const auto pSourceBufferMTL = pSourceBuffer.Downcast<spBufferMTL>();
    const auto pDestinationBufferMTL = pDestinationBuffer.Downcast<spBufferMTL>();
    pSourceBufferMTL->EnsureResourceCreated();
    pDestinationBufferMTL->EnsureResourceCreated();

    auto* pDevice = static_cast<spDeviceMTL*>(m_pDevice);

    if (uiSourceOffset % 4 != 0 || uiDestinationOffset % 4 != 0 || uiSize % 4 != 0)
    {
      // Unaligned copy -- use special compute shader.
      EnsureComputeEncoder();
      m_pComputeCommandEncoder->setComputePipelineState(pDevice->GetUnalignedBufferCopyComputePipelineState());
      m_pComputeCommandEncoder->setBuffer(pSourceBufferMTL->GetMTLBuffer(), 0, 0);
      m_pComputeCommandEncoder->setBuffer(pDestinationBufferMTL->GetMTLBuffer(), 0, 1);

      spUnalignedBufferCopyData copyInfo{};
      copyInfo.m_uiSourceOffset = uiSourceOffset;
      copyInfo.m_uiDestinationOffset = uiDestinationOffset;
      copyInfo.m_uiSize = uiSize;

      m_pComputeCommandEncoder->setBytes(&copyInfo, sizeof(spUnalignedBufferCopyData), 2);
      m_pComputeCommandEncoder->dispatchThreadgroups(MTL::Size(1, 1, 1), MTL::Size(1, 1, 1));
    }
  }

  void spCommandListMTL::CopyTextureInternal(ezSharedPtr<spTexture> pSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, ezSharedPtr<spTexture> pDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount)
  {
    EnsureBlitEncoder();
    auto pSourceTextureMTL = pSourceTexture.Downcast<spTextureMTL>();
    auto pDestinationTextureMTL = pDestinationTexture.Downcast<spTextureMTL>();

    pSourceTextureMTL->EnsureResourceCreated();
    pDestinationTextureMTL->EnsureResourceCreated();

    bool bSourceIsStaging = pSourceTextureMTL->GetUsage().IsSet(spTextureUsage::Staging);
    bool bDestinationIsStaging = pDestinationTextureMTL->GetUsage().IsSet(spTextureUsage::Staging);

    if (bSourceIsStaging && !bDestinationIsStaging)
    {
      auto* pSrcBuffer = pSourceTextureMTL->GetMTLStagingBuffer();
      auto* pDstTexture = pDestinationTextureMTL->GetMTLTexture();

      ezUInt32 uiMipWidth = 0, uiMipHeight = 0, uiMipDepth = 0;
      spTextureHelper::GetMipDimensions(pSourceTexture, uiSourceMipLevel, uiMipWidth, uiMipHeight, uiMipDepth);

      for (ezUInt32 layer = 0; layer < uiLayerCount; ++layer)
      {
        ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(pSourceTextureMTL->GetFormat()) ? 4 : 1;
        ezUInt32 uiCompressedSrcX = uiSourceX / uiBlockSize;
        ezUInt32 uiCompressedSrcY = uiSourceY / uiBlockSize;

        uint blockSizeInBytes = uiBlockSize == 1
                                  ? spPixelFormatHelper::GetSizeInBytes(pSourceTextureMTL->GetFormat())
                                  : spPixelFormatHelper::GetBlockSizeInBytes(pSourceTextureMTL->GetFormat());

        ezUInt32 uiSourceSubresourceBase = spTextureHelper::CalculateSubresource(
          pSourceTextureMTL,
          uiSourceMipLevel,
          layer + uiSourceBaseArrayLayer);

        ezUInt32 uiSourceRowPitch, uiSourceDepthPitch;
        pSourceTextureMTL->GetSubresourceLayout(
          uiSourceMipLevel,
          uiSourceBaseArrayLayer + layer,
          uiSourceRowPitch,
          uiSourceDepthPitch);

        ezUInt32 uiSourceOffset = uiSourceSubresourceBase + uiSourceDepthPitch * uiSourceZ + uiSourceRowPitch * uiCompressedSrcY + blockSizeInBytes * uiCompressedSrcX;

        ezUInt32 uiCopyWidth = uiWidth > uiMipWidth && uiWidth <= uiBlockSize
                                 ? uiMipWidth
                                 : uiWidth;

        ezUInt32 uiCopyHeight = uiHeight > uiMipHeight && uiHeight <= uiBlockSize
                                  ? uiMipHeight
                                  : uiHeight;

        MTL::Size sourceSize = MTL::Size(uiCopyWidth, uiCopyHeight, uiDepth);

        if (pDestinationTextureMTL->GetDimension() != spTextureDimension::Texture3D)
        {
          uiSourceDepthPitch = 0;
        }

        m_pBlitCommandEncoder->copyFromBuffer(
          pSrcBuffer,
          uiSourceOffset,
          uiSourceRowPitch,
          uiSourceDepthPitch,
          sourceSize,
          pDstTexture,
          (uiDestBaseArrayLayer + layer),
          uiDestMipLevel,
          MTL::Origin(uiDestX, uiDestY, uiDestZ));
      }
    }
    else if (bSourceIsStaging && bDestinationIsStaging)
    {
      for (ezUInt32 layer = 0; layer < uiLayerCount; layer++)
      {
        ezUInt32 uiSourceSubresourceBase = spTextureHelper::CalculateSubresource(
          pSourceTextureMTL,
          uiSourceMipLevel,
          layer + uiSourceBaseArrayLayer);

        ezUInt32 uiSourceRowPitch, uiSourceDepthPitch;
        pSourceTextureMTL->GetSubresourceLayout(
          uiSourceMipLevel,
          uiSourceBaseArrayLayer + layer,
          uiSourceRowPitch,
          uiSourceDepthPitch);

        ezUInt32 uiDestinationSubresourceBase = spTextureHelper::CalculateSubresource(
          pDestinationTextureMTL,
          uiDestMipLevel,
          layer + uiDestBaseArrayLayer);

        ezUInt32 uiDestinationRowPitch, uiDestinationDepthPitch;
        pDestinationTextureMTL->GetSubresourceLayout(
          uiDestMipLevel,
          uiDestBaseArrayLayer + layer,
          uiDestinationRowPitch,
          uiDestinationDepthPitch);

        ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(pDestinationTextureMTL->GetFormat()) ? 4 : 1;

        if (uiBlockSize == 1)
        {
          ezUInt32 uiPixelSize = spPixelFormatHelper::GetSizeInBytes(pDestinationTextureMTL->GetFormat());
          ezUInt32 uiCopySize = uiWidth * uiPixelSize;

          for (ezUInt32 zz = 0; zz < uiDepth; zz++)
          {
            for (ezUInt32 yy = 0; yy < uiHeight; yy++)
            {
              ezUInt32 srcRowOffset = uiSourceSubresourceBase + uiSourceDepthPitch * (zz + uiSourceZ) + uiSourceRowPitch * (yy + uiSourceY) + uiPixelSize * uiSourceX;
              ezUInt32 dstRowOffset = uiDestinationSubresourceBase + uiDestinationDepthPitch * (zz + uiDestZ) + uiDestinationRowPitch * (yy + uiDestY) + uiPixelSize * uiDestX;

              m_pBlitCommandEncoder->copyFromBuffer(
                pSourceTextureMTL->GetMTLStagingBuffer(),
                srcRowOffset,
                pDestinationTextureMTL->GetMTLStagingBuffer(),
                dstRowOffset,
                uiCopySize);
            }
          }
        }
        else // blockSize != 1
        {
          ezUInt32 uiPaddedWidth = ezMath::Max(uiBlockSize, uiWidth);
          ezUInt32 paddedHeight = ezMath::Max(uiBlockSize, uiHeight);
          ezUInt32 uiNumRows = spPixelFormatHelper::GetNumRows(paddedHeight, pSourceTextureMTL->GetFormat());
          ezUInt32 uiRowPitch = spPixelFormatHelper::GetRowPitch(uiPaddedWidth, pSourceTextureMTL->GetFormat());

          ezUInt32 uiCompressedSrcX = uiSourceX / 4;
          ezUInt32 uiCompressedSrcY = uiSourceY / 4;
          ezUInt32 uiCompressedDstX = uiDestX / 4;
          ezUInt32 uiCompressedDstY = uiDestY / 4;
          ezUInt32 uiBlockSizeInBytes = spPixelFormatHelper::GetBlockSizeInBytes(pSourceTextureMTL->GetFormat());

          for (ezUInt32 zz = 0; zz < uiDepth; zz++)
          {
            for (ezUInt32 row = 0; row < uiNumRows; row++)
            {
              ezUInt32 srcRowOffset = uiSourceSubresourceBase + uiSourceDepthPitch * (zz + uiSourceZ) + uiSourceRowPitch * (row + uiCompressedSrcY) + uiBlockSizeInBytes * uiCompressedSrcX;
              ezUInt32 dstRowOffset = uiDestinationSubresourceBase + uiDestinationDepthPitch * (zz + uiDestZ) + uiDestinationRowPitch * (row + uiCompressedDstY) + uiBlockSizeInBytes * uiCompressedDstX;

              m_pBlitCommandEncoder->copyFromBuffer(
                pSourceTextureMTL->GetMTLStagingBuffer(),
                srcRowOffset,
                pDestinationTextureMTL->GetMTLStagingBuffer(),
                dstRowOffset,
                uiRowPitch);
            }
          }
        }
      }
    }
    else if (!bSourceIsStaging && bDestinationIsStaging)
    {
      auto srcOrigin = MTL::Origin(uiSourceX, uiSourceY, uiSourceZ);
      auto srcSize = MTL::Size(uiWidth, uiHeight, uiDepth);

      for (ezUInt32 layer = 0; layer < uiLayerCount; layer++)
      {
        ezUInt32 uiDestinationBytesPerRow, uiDestinationBytesPerImage;
        pDestinationTextureMTL->GetSubresourceLayout(
          uiDestMipLevel,
          uiDestBaseArrayLayer + layer,
          uiDestinationBytesPerRow,
          uiDestinationBytesPerImage);

        ezUInt32 uiMipWidth, uiMipHeight, uiMipDepth;
        spTextureHelper::GetMipDimensions(pDestinationTextureMTL, uiDestMipLevel, uiMipWidth, uiMipHeight, uiMipDepth);

        ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(pSourceTextureMTL->GetFormat()) ? 4 : 1;
        ezUInt32 uiBufferRowLength = ezMath::Max(uiMipWidth, uiBlockSize);
        ezUInt32 uiBufferImageHeight = ezMath::Max(uiMipHeight, uiBlockSize);
        ezUInt32 uiCompressedDstX = uiDestX / uiBlockSize;
        ezUInt32 uiCompressedDstY = uiDestY / uiBlockSize;
        ezUInt32 uiBlockSizeInBytes = uiBlockSize == 1
                                        ? spPixelFormatHelper::GetSizeInBytes(pSourceTextureMTL->GetFormat())
                                        : spPixelFormatHelper::GetBlockSizeInBytes(pSourceTextureMTL->GetFormat());
        ezUInt32 uiRowPitch = spPixelFormatHelper::GetRowPitch(uiBufferRowLength, pSourceTextureMTL->GetFormat());
        ezUInt32 uiDepthPitch = spPixelFormatHelper::GetDepthPitch(uiRowPitch, uiBufferImageHeight, pSourceTextureMTL->GetFormat());

        ezUInt32 dstOffset = spTextureHelper::CalculateSubresource(pDestinationTextureMTL, uiDestMipLevel, uiDestBaseArrayLayer + layer) + (uiDestZ * uiDepthPitch) + (uiCompressedDstY * uiRowPitch) + (uiCompressedDstX * uiBlockSizeInBytes);

        m_pBlitCommandEncoder->copyFromTexture(
          pSourceTextureMTL->GetMTLTexture(),
          (uiSourceBaseArrayLayer + layer),
          uiSourceMipLevel,
          srcOrigin,
          srcSize,
          pDestinationTextureMTL->GetMTLStagingBuffer(),
          dstOffset,
          uiDestinationBytesPerRow,
          uiDestinationBytesPerImage);
      }
    }
    else
    {
      for (uint layer = 0; layer < uiLayerCount; layer++)
      {
        m_pBlitCommandEncoder->copyFromTexture(
          pSourceTextureMTL->GetMTLTexture(),
          (uiSourceBaseArrayLayer + layer),
          uiSourceMipLevel,
          MTL::Origin(uiSourceX, uiSourceY, uiSourceZ),
          MTL::Size(uiWidth, uiHeight, uiDepth),
          pDestinationTextureMTL->GetMTLTexture(),
          (uiDestBaseArrayLayer + layer),
          uiDestMipLevel,
          MTL::Origin(uiDestX, uiDestY, uiDestZ));
      }
    }
  }

  void spCommandListMTL::GenerateMipmapsInternal(ezSharedPtr<spTexture> pTexture)
  {
    EZ_ASSERT_DEV(pTexture->GetMipCount() > 1, "Texture must have at least two mip levels to generate mipmaps.");

    EnsureBlitEncoder();
    const auto pTextureMTL = static_cast<spTextureMTL*>(pTexture.Borrow());
    pTextureMTL->EnsureResourceCreated();

    m_pBlitCommandEncoder->generateMipmaps(pTextureMTL->GetMTLTexture());
  }

  spCommandListMTL::spCommandListMTL(spDeviceMTL* pDeviceMTL, const spCommandListDescription& description)
    : spCommandList(description)
  {
    m_pDevice = pDeviceMTL;
    m_pMTLDevice = pDeviceMTL->GetMTLDevice();

    m_bReleased = false;
  }

  spCommandListMTL::~spCommandListMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  MTL::CommandBuffer* spCommandListMTL::Commit()
  {
    m_pCommandBuffer->commit();
    auto* pCommandBuffer = m_pCommandBuffer;
    m_pCommandBuffer = nullptr;

    return pCommandBuffer;
  }

  void spCommandListMTL::SetCompletionFence(ezSharedPtr<spFenceMTL> pFence)
  {
    EZ_ASSERT_DEV(m_CompletionFence == nullptr, "Completion fence already set.");
    m_CompletionFence = pFence;
  }

  void spCommandListMTL::OnCompleted(MTL::CommandBuffer* pCommandBuffer)
  {
    if (m_CompletionFence != nullptr)
    {
      m_CompletionFence->Raise();
      m_CompletionFence.Clear();
    }

    {
      EZ_LOCK(m_SubmittedCommandsLock);

      ezDynamicArray<ezSharedPtr<spBufferMTL>>* stagingBuffers = nullptr;
      if (m_SubmittedStagingBuffers.TryGetValue(pCommandBuffer, stagingBuffers))
      {
        m_AvailableStagingBuffers.InsertRange(stagingBuffers->GetArrayPtr(), 0);
        m_SubmittedStagingBuffers.Remove(pCommandBuffer);
      }
    }
  }

  bool spCommandListMTL::IsResourceSetEqual(spCommandListResourceSet& set, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets)
  {
    return set.m_hResourceSet == pResourceSet->GetHandle() && ((set.m_Offsets.GetPtr() == pDynamicOffsets && set.m_Offsets.GetCount() == uiDynamicOffsetCount) || ezMemoryUtils::IsEqual(set.m_Offsets.GetPtr(), pDynamicOffsets, uiDynamicOffsetCount));
  }

  bool spCommandListMTL::PreDraw()
  {
    if (!EnsureRenderPass())
      return false;

    const auto pGraphicPipelineMTL = m_pGraphicPipeline.Downcast<spGraphicPipelineMTL>();
    EZ_ASSERT_DEV(pGraphicPipelineMTL != nullptr, "No graphics pipeline has been set.");

    if (m_bViewportsChanged)
    {
      FlushViewports();
      m_bViewportsChanged = false;
    }

    if (m_bScissorRectsChanged && pGraphicPipelineMTL->IsScissorTestEnabled())
    {
      FlushScissorRects();
      m_bScissorRectsChanged = false;
    }

    if (m_bGraphicPipelineChanged)
    {
      m_pRenderCommandEncoder->setRenderPipelineState(pGraphicPipelineMTL->GetPipelineState());
      m_pRenderCommandEncoder->setCullMode(pGraphicPipelineMTL->GetCullMode());
      m_pRenderCommandEncoder->setFrontFacingWinding(pGraphicPipelineMTL->GetWinding());
      m_pRenderCommandEncoder->setTriangleFillMode(pGraphicPipelineMTL->GetFillMode());
      m_pRenderCommandEncoder->setDepthBias(pGraphicPipelineMTL->GetDepthBias(), pGraphicPipelineMTL->GetSlopeScaledDepthBias(), pGraphicPipelineMTL->GetDepthBiasClamp());

      const ezColor& blendColor = pGraphicPipelineMTL->GetBlendColor();
      m_pRenderCommandEncoder->setBlendColor(blendColor.r, blendColor.g, blendColor.b, blendColor.a);

      if (!m_pFramebuffer->GetDepthTarget().IsInvalidated())
      {
        m_pRenderCommandEncoder->setDepthStencilState(pGraphicPipelineMTL->GetDepthStencilState());
        m_pRenderCommandEncoder->setDepthClipMode(pGraphicPipelineMTL->GetDepthClipMode());
        m_pRenderCommandEncoder->setStencilReferenceValue(pGraphicPipelineMTL->GetStencilReference());
      }

      m_bGraphicPipelineChanged = false;
    }

    for (ezUInt32 i = 0, l = pGraphicPipelineMTL->GetResourceLayouts().GetCount(); i < l; i++)
    {
      if (m_ActiveGraphicResourceSets[i])
        continue;

      ActivateGraphicResourceSet(i, m_GraphicResourceSets[i]);
      m_ActiveGraphicResourceSets[i] = true;
    }

    for (ezUInt32 i = 0, l = pGraphicPipelineMTL->GetVertexBufferCount(); i < l; i++)
    {
      if (m_ActiveVertexBuffers[i])
        continue;

      m_pRenderCommandEncoder->setVertexBuffer(
        m_VertexBuffers[i]->GetMTLBuffer(),
        m_VertexOffsets[i],
        m_GraphicResourceSets.GetCount() + i);

      m_ActiveVertexBuffers[i] = true;
    }

    if (pGraphicPipelineMTL->SupportsPushConstants() && m_PushConstant.m_pData != nullptr)
    {
      if (m_PushConstant.m_eStage.IsSet(spShaderStage::VertexShader))
      {
        m_pRenderCommandEncoder->setVertexBytes(
          static_cast<const ezUInt8*>(m_PushConstant.m_pData) + m_PushConstant.m_uiOffset,
          m_PushConstant.m_uiSize,
          0);
      }
      else if (m_PushConstant.m_eStage.IsSet(spShaderStage::PixelShader))
      {
        m_pRenderCommandEncoder->setFragmentBytes(
          static_cast<const ezUInt8*>(m_PushConstant.m_pData) + m_PushConstant.m_uiOffset,
          m_PushConstant.m_uiSize,
          0);
      }
    }

    return true;
  }

  void spCommandListMTL::PreDispatch()
  {
    EnsureComputeEncoder();

    const auto pComputePipelineMTL = m_pComputePipeline.Downcast<spComputePipelineMTL>();

    if (m_bComputePipelineChanged)
      m_pComputeCommandEncoder->setComputePipelineState(pComputePipelineMTL->GetPipelineState());

    for (uint i = 0, l = pComputePipelineMTL->GetResourceLayouts().GetCount(); i < l; i++)
    {
      if (m_ActiveComputeResourceSets[i])
        continue;

      ActivateComputeResourceSet(i, m_ComputeResourceSets[i]);
      m_ActiveComputeResourceSets[i] = true;
    }

    if (m_pComputePipeline->SupportsPushConstants() && m_PushConstant.m_pData != nullptr)
    {
      if (m_PushConstant.m_eStage.IsSet(spShaderStage::ComputeShader))
      {
        m_pComputeCommandEncoder->setBytes(
          static_cast<const ezUInt8*>(m_PushConstant.m_pData) + m_PushConstant.m_uiOffset,
          m_PushConstant.m_uiSize,
          0);
      }
    }
  }

  void spCommandListMTL::EnsureNoRenderPass()
  {
    if (IsRenderCommandEncoderActive())
    {
      EndCurrentRenderPass();
    }

    EZ_ASSERT_DEV(!IsRenderCommandEncoderActive(), "Invalid state. The render encoder is active.");
  }

  void spCommandListMTL::EndCurrentRenderPass()
  {
    m_pRenderCommandEncoder->endEncoding();
    SP_RHI_MTL_RELEASE(m_pRenderCommandEncoder);

    ezMemoryUtils::ZeroFill(m_ActiveGraphicResourceSets.GetData(), m_ActiveGraphicResourceSets.GetCount());

    m_bGraphicPipelineChanged = true;
    m_bViewportsChanged = true;
    m_bScissorRectsChanged = true;
  }

  void spCommandListMTL::EnsureNoBlitEncoder()
  {
    if (IsBlitCommandEncoderActive())
    {
      m_pBlitCommandEncoder->endEncoding();
      SP_RHI_MTL_RELEASE(m_pBlitCommandEncoder);
    }

    EZ_ASSERT_DEV(!IsBlitCommandEncoderActive(), "Invalid state. The blit encoder is active.");
  }

  void spCommandListMTL::EnsureNoComputeEncoder()
  {
    if (IsComputeCommandEncoderActive())
    {
      m_pComputeCommandEncoder->endEncoding();
      SP_RHI_MTL_RELEASE(m_pComputeCommandEncoder);

      m_bComputePipelineChanged = true;
      ezMemoryUtils::ZeroFill(m_ActiveComputeResourceSets.GetData(), m_ActiveComputeResourceSets.GetCount());
    }

    EZ_ASSERT_DEV(!IsComputeCommandEncoderActive(), "Invalid state. The compute encoder is active.");
  }

  bool spCommandListMTL::EnsureRenderPass()
  {
    EZ_ASSERT_DEV(m_pFramebuffer != nullptr, "No framebuffer has been set.");

    EnsureNoBlitEncoder();
    EnsureNoComputeEncoder();

    return IsRenderCommandEncoderActive() || BeginCurrentRenderPass();
  }

  void spCommandListMTL::EnsureBlitEncoder()
  {
    if (!IsBlitCommandEncoderActive())
    {
      EnsureNoRenderPass();
      EnsureNoComputeEncoder();

      {
        spScopedMTLResource autoReleasePool(NS::AutoreleasePool::alloc()->init());
        m_pBlitCommandEncoder = m_pCommandBuffer->blitCommandEncoder();
        SP_RHI_MTL_RETAIN(m_pBlitCommandEncoder);
      }

      if (m_bIsInDebugGroup)
      {
        ezStringBuilder sb;
        spScopedMTLResource nsName(NS::String::string(m_sDebugGroupName.GetData(sb), NS::UTF8StringEncoding));

        m_pBlitCommandEncoder->pushDebugGroup(*nsName);
      }
    }

    EZ_ASSERT_DEV(IsBlitCommandEncoderActive(), "Invalid state. The blit encoder is not active.");
    EZ_ASSERT_DEV(!IsRenderCommandEncoderActive(), "Invalid state. The render encoder is active.");
    EZ_ASSERT_DEV(!IsComputeCommandEncoderActive(), "Invalid state. The compute encoder is active.");
  }

  void spCommandListMTL::EnsureComputeEncoder()
  {
    if (!IsComputeCommandEncoderActive())
    {
      EnsureNoBlitEncoder();
      EnsureNoRenderPass();

      {
        spScopedMTLResource autoReleasePool(NS::AutoreleasePool::alloc()->init());
        m_pComputeCommandEncoder = m_pCommandBuffer->computeCommandEncoder();
        SP_RHI_MTL_RETAIN(m_pComputeCommandEncoder);
      }

      if (m_bIsInDebugGroup)
      {
        ezStringBuilder sb;
        spScopedMTLResource nsName(NS::String::string(m_sDebugGroupName.GetData(sb), NS::UTF8StringEncoding));

        m_pComputeCommandEncoder->pushDebugGroup(*nsName);
      }
    }

    EZ_ASSERT_DEV(IsComputeCommandEncoderActive(), "Invalid state. The compute encoder is not active.");
    EZ_ASSERT_DEV(!IsRenderCommandEncoderActive(), "Invalid state. The render encoder is active.");
    EZ_ASSERT_DEV(!IsBlitCommandEncoderActive(), "Invalid state. The blit encoder is active.");
  }

  bool spCommandListMTL::HasAnyUnsetClearValues()
  {
    // If we have valid clear values, we need to end the current render pass.
    bool bShouldEndRenderPass = ezMath::IsFinite(m_fClearDepth);
    for (const auto& clearColor : m_ClearColors)
      bShouldEndRenderPass |= clearColor.IsValid();

    return bShouldEndRenderPass;
  }

  bool spCommandListMTL::BeginCurrentRenderPass()
  {
    if (!m_pFramebuffer->IsRenderable())
      return false;

    MTL::RenderPassDescriptor* rpDesc = m_pFramebuffer->GetRenderPassDescriptor();

    for (ezUInt32 i = 0, l = m_ClearColors.GetCount(); i < l; i++)
    {
      if (m_ClearColors[i].IsValid())
      {
        auto* attachment = rpDesc->colorAttachments()->object(i);
        attachment->setLoadAction(MTL::LoadActionClear);
        attachment->setStoreAction(MTL::StoreActionStore);
        const ezColor& c = m_ClearColors[i];
        attachment->setClearColor(MTL::ClearColor(c.r, c.g, c.b, c.a));
        m_ClearColors[i] = ezColor::MakeNaN();
      }
    }

    if (ezMath::IsFinite(m_fClearDepth))
    {
      MTL::RenderPassDepthAttachmentDescriptor* depthAttachment = rpDesc->depthAttachment();
      depthAttachment->setLoadAction(MTL::LoadActionClear);
      depthAttachment->setStoreAction(MTL::StoreActionStore);
      depthAttachment->setClearDepth(m_fClearDepth);

      const auto pTexture = m_pDevice->GetResourceManager()->GetResource<spTexture>(m_pFramebuffer->GetDepthTarget());
      EZ_ASSERT_DEV(pTexture != nullptr, "Trying to get an unregistered texture from the device.");

      if (spPixelFormatHelper::IsStencilFormat(pTexture->GetFormat()))
      {
        MTL::RenderPassStencilAttachmentDescriptor* stencilAttachment = rpDesc->stencilAttachment();
        stencilAttachment->setLoadAction(MTL::LoadActionClear);
        stencilAttachment->setStoreAction(MTL::StoreActionStore);
        stencilAttachment->setClearStencil(m_uiClearStencil);
      }

      m_fClearDepth = ezMath::NaN<float>();
    }

    {
      spScopedMTLResource autoReleasePool(NS::AutoreleasePool::alloc()->init());
      m_pRenderCommandEncoder = m_pCommandBuffer->renderCommandEncoder(rpDesc);
      SP_RHI_MTL_RETAIN(m_pRenderCommandEncoder);
    }

    m_bCurrentFramebufferEverActive = true;

    if (m_bIsInDebugGroup)
    {
      ezStringBuilder sb;
      spScopedMTLResource nsName(NS::String::string(m_sDebugGroupName.GetData(sb), NS::UTF8StringEncoding));

      m_pRenderCommandEncoder->pushDebugGroup(*nsName);
    }

    return true;
  }

  void spCommandListMTL::FlushViewports()
  {
    const auto* pDeviceMTL = static_cast<spDeviceMTL*>(m_pDevice);

    if (pDeviceMTL->GetSupportedFeatures().IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v3))
    {
      m_pRenderCommandEncoder->setViewports(m_Viewports.GetData(), m_Viewports.GetCount());
    }
    else
    {
      m_pRenderCommandEncoder->setViewport(m_Viewports[0]);
    }
  }

  void spCommandListMTL::FlushScissorRects()
  {
    const auto* pDeviceMTL = static_cast<spDeviceMTL*>(m_pDevice);

    if (pDeviceMTL->GetSupportedFeatures().IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v3))
    {
      m_pRenderCommandEncoder->setScissorRects(m_ScissorRects.GetData(), m_ScissorRects.GetCount());
    }
    else
    {
      m_pRenderCommandEncoder->setScissorRect(m_ScissorRects[0]);
    }
  }

  void spCommandListMTL::ClearBoundResources()
  {
    m_BoundResources.Clear();
    m_ArgumentEncoders.Clear();
    m_ArgumentBuffers.Clear();
  }

  void spCommandListMTL::EnsureArgumentBuffer(ezUInt32 uiSlot, ezSharedPtr<spShaderProgramMTL> pProgram, ezSharedPtr<spResourceSetMTL> pResourceSet, ezEnum<spShaderStage> eStage)
  {
    const ezUInt32 uiKey = GetResourceSetKey(uiSlot, eStage);

    ezSharedPtr<spShaderMTL> pShader;

    if (eStage == spShaderStage::VertexShader)
      pShader = pProgram->GetVertexShader();
    else if (eStage == spShaderStage::PixelShader)
      pShader = pProgram->GetPixelShader();
    else if (eStage == spShaderStage::ComputeShader)
      pShader->EnsureResourceCreated();

    bool bShouldSet = false;

    MTL::ArgumentEncoder* pArgumentEncoder = nullptr;
    if (const ezUInt32 uiIndex = m_ArgumentEncoders.Find(uiKey); uiIndex == ezInvalidIndex)
    {
      pArgumentEncoder = pShader->GetArgumentEncoder(uiSlot);
      m_ArgumentEncoders[uiKey] = pArgumentEncoder;

      bShouldSet = true;
    }

    ezSharedPtr<spBufferMTL> pArgumentBuffer = nullptr;
    if (const ezUInt32 uiIndex = m_ArgumentBuffers.Find(uiKey); uiIndex == ezInvalidIndex)
    {
      pArgumentBuffer = pResourceSet->GetArgumentBuffer(pArgumentEncoder);
      m_ArgumentBuffers[uiKey] = pArgumentBuffer;

      bShouldSet = true;
    }

    if (bShouldSet)
    {
      pArgumentBuffer->EnsureResourceCreated();
      pArgumentEncoder->setArgumentBuffer(pArgumentBuffer->GetMTLBuffer(), 0);
    }
  }

  void spCommandListMTL::BindArgumentBuffer(ezUInt32 uiSlot, ezEnum<spShaderStage> eStage)
  {
    const ezUInt32 uiKey = GetResourceSetKey(uiSlot, eStage);
    const ezUInt32 uiIndex = m_ArgumentBuffers.Find(uiKey);

    if (uiIndex == ezInvalidIndex)
      return;

    const auto pArgumentBuffer = m_ArgumentBuffers.GetValue(uiIndex);
    const BoundResource resource{pArgumentBuffer->GetHandle(), uiSlot, 0, eStage};

    if (m_BoundResources[uiKey] == resource)
      return;

    m_BoundResources[uiKey] = resource;

    if (eStage == spShaderStage::VertexShader)
      m_pRenderCommandEncoder->setVertexBuffer(pArgumentBuffer->GetMTLBuffer(), 0, uiSlot);
    else if (eStage == spShaderStage::PixelShader)
      m_pRenderCommandEncoder->setFragmentBuffer(pArgumentBuffer->GetMTLBuffer(), 0, uiSlot);
    else if (eStage == spShaderStage::ComputeShader)
      m_pComputeCommandEncoder->setBuffer(pArgumentBuffer->GetMTLBuffer(), 0, uiSlot);
  }

  void spCommandListMTL::ActivateGraphicResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet)
  {
    EZ_ASSERT_DEV(IsRenderCommandEncoderActive(), "Invalid state. The render encoder is not active.");

    const auto pResourceSetMTL = m_pDevice->GetResourceManager()->GetResource<spResourceSetMTL>(resourceSet.m_hResourceSet);
    const auto pResourceLayoutMTL = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutMTL>(pResourceSetMTL->GetLayout());

    const auto pProgram = m_pDevice->GetResourceManager()->GetResource<spShaderProgramMTL>(m_pGraphicPipeline->GetShaderProgram());
    EZ_ASSERT_DEV(pProgram != nullptr, "Invalid shader program handle {0} in graphic pipeline", m_pGraphicPipeline->GetShaderProgram().GetInternalID().m_Data);

    if (m_pGraphicPipeline->SupportsPushConstants())
      uiSlot++;

    const std::initializer_list<spShaderStage::Enum> stages = {spShaderStage::VertexShader, spShaderStage::PixelShader};

    bool bShouldActivate = false;

    for (const auto& stage : stages)
    {
      if (!pResourceLayoutMTL->GetShaderStages().IsSet(stage))
        continue;

      bShouldActivate = true;
      EnsureArgumentBuffer(uiSlot, pProgram, pResourceSetMTL, stage);
    }

    if (bShouldActivate)
    {
      ActivateResourceSet(uiSlot, resourceSet);

      for (const auto& stage : stages)
      {
        if (!pResourceLayoutMTL->GetShaderStages().IsSet(stage))
          continue;

        BindArgumentBuffer(uiSlot, stage);
      }
    }
  }

  void spCommandListMTL::ActivateComputeResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet)
  {
    EZ_ASSERT_DEV(IsComputeCommandEncoderActive(), "Invalid state. The compute encoder is not active.");

    const auto pResourceSetMTL = m_pDevice->GetResourceManager()->GetResource<spResourceSetMTL>(resourceSet.m_hResourceSet);
    const auto pResourceLayoutMTL = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutMTL>(pResourceSetMTL->GetLayout());

    const auto pProgram = m_pDevice->GetResourceManager()->GetResource<spShaderProgramMTL>(m_pGraphicPipeline->GetShaderProgram());
    EZ_ASSERT_DEV(pProgram != nullptr, "Invalid shader program handle {0} in graphic pipeline", m_pGraphicPipeline->GetShaderProgram().GetInternalID().m_Data);

    if (!pResourceLayoutMTL->GetShaderStages().IsSet(spShaderStage::ComputeShader))
      return;

    EnsureArgumentBuffer(uiSlot, pProgram, pResourceSetMTL, spShaderStage::ComputeShader);
    ActivateResourceSet(uiSlot, resourceSet);
    BindArgumentBuffer(uiSlot, spShaderStage::ComputeShader);
  }

  void spCommandListMTL::ActivateResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet)
  {
    const auto pResourceSetMTL = m_pDevice->GetResourceManager()->GetResource<spResourceSetMTL>(resourceSet.m_hResourceSet);
    const auto pResourceLayoutMTL = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutMTL>(pResourceSetMTL->GetLayout());

    ezUInt32 uiDynamicOffsetIndex = 0;

    const auto& elements = pResourceLayoutMTL->GetElements();
    for (ezUInt32 i = 0, l = elements.GetCount(); i < l; i++)
    {
      const auto& sName = elements[i].m_sName;
      const auto& bindingInfo = pResourceLayoutMTL->GetBinding(i);
      const auto& hResource = pResourceSetMTL->GetBoundResource(sName);

      ezUInt32 uiBufferOffset = 0;
      if (bindingInfo.m_bDynamicBuffer)
      {
        uiBufferOffset = resourceSet.m_Offsets[uiDynamicOffsetIndex];
        uiDynamicOffsetIndex += 1;
      }

      switch (bindingInfo.m_eResourceType)
      {
        case spShaderResourceType::ConstantBuffer:
        case spShaderResourceType::ReadOnlyStructuredBuffer:
        case spShaderResourceType::ReadWriteStructuredBuffer:
        {
          const auto pRange = spResourceHelper::GetBufferRange(m_pDevice, hResource, uiBufferOffset).Downcast<spBufferRangeMTL>();
          BindBuffer(pRange, uiSlot, bindingInfo.m_uiSlot, bindingInfo.m_eShaderStage);
          break;
        }

        case spShaderResourceType::ReadWriteTexture:
        case spShaderResourceType::ReadOnlyTexture:
        {
          const auto pTexView = spTextureSamplerManager::GetTextureView(m_pDevice, hResource).Downcast<spTextureViewMTL>();
          BindTexture(pTexView, uiSlot, bindingInfo.m_uiSlot, bindingInfo.m_eShaderStage);
          break;
        }

        case spShaderResourceType::Sampler:
        {
          const auto pSampler = m_pDevice->GetResourceManager()->GetResource<spSamplerMTL>(hResource);
          BindSampler(pSampler, uiSlot, bindingInfo.m_uiSlot, bindingInfo.m_eShaderStage);
          break;
        }

        default:
          EZ_ASSERT_NOT_IMPLEMENTED;
          break;
      }
    }
  }

  void spCommandListMTL::BindBuffer(ezSharedPtr<spBufferRangeMTL> pBuffer, ezUInt32 uiSet, ezUInt32 uiSlot, ezBitflags<spShaderStage> eStages)
  {
    pBuffer->EnsureResourceCreated();
    const auto* pMTLBuffer = pBuffer->GetBuffer().Downcast<spBufferMTL>()->GetMTLBuffer();

    if (eStages.IsSet(spShaderStage::ComputeShader))
    {
      MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::ComputeShader)];
      EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind buffer to compute kernel. The argument buffer encoder is invalid.");

      pArgumentEncoder->setBuffer(pMTLBuffer, pBuffer->GetOffset(), uiSlot);
      m_pComputeCommandEncoder->useResource(pMTLBuffer, spToMTL(pBuffer->GetBuffer()->GetUsage()));
    }
    else
    {
      if (eStages.IsSet(spShaderStage::VertexShader))
      {
        MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::VertexShader)];
        EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind buffer to vertex kernel. The argument buffer encoder is invalid.");

        pArgumentEncoder->setBuffer(pMTLBuffer, pBuffer->GetOffset(), uiSlot);
        m_pRenderCommandEncoder->useResource(pMTLBuffer, spToMTL(pBuffer->GetBuffer()->GetUsage()), MTL::RenderStageVertex);
      }

      if (eStages.IsSet(spShaderStage::PixelShader))
      {
        MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::PixelShader)];
        EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind buffer to fragment kernel. The argument buffer encoder is invalid.");

        pArgumentEncoder->setBuffer(pMTLBuffer, pBuffer->GetOffset(), uiSlot);
        m_pRenderCommandEncoder->useResource(pMTLBuffer, spToMTL(pBuffer->GetBuffer()->GetUsage()), MTL::RenderStageFragment);
      }
    }
  }

  void spCommandListMTL::BindTexture(ezSharedPtr<spTextureViewMTL> pTextureView, ezUInt32 uiSet, ezUInt32 uiSlot, ezBitflags<spShaderStage> eStages)
  {
    pTextureView->EnsureResourceCreated();
    const auto pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureMTL>(pTextureView->GetTexture());
    const MTL::Texture* pMTLTexture = pTextureView->GetMTLTargetTexture();

    if (eStages.IsSet(spShaderStage::ComputeShader))
    {
      MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::ComputeShader)];
      EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind texture to compute kernel. The argument buffer encoder is invalid.");

      pArgumentEncoder->setTexture(pMTLTexture, uiSlot);
      m_pComputeCommandEncoder->useResource(pMTLTexture, spToMTLResourceUsage(pTexture->GetUsage()));
    }
    else
    {
      if (eStages.IsSet(spShaderStage::VertexShader))
      {
        MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::VertexShader)];
        EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind texture to vertex kernel. The argument buffer encoder is invalid.");

        pArgumentEncoder->setTexture(pMTLTexture, uiSlot);
        m_pRenderCommandEncoder->useResource(pMTLTexture, spToMTLResourceUsage(pTexture->GetUsage()));
      }

      if (eStages.IsSet(spShaderStage::PixelShader))
      {
        MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::PixelShader)];
        EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind texture to fragment kernel. The argument buffer encoder is invalid.");

        pArgumentEncoder->setTexture(pMTLTexture, uiSlot);
        m_pRenderCommandEncoder->useResource(pMTLTexture, spToMTLResourceUsage(pTexture->GetUsage()));
      }
    }
  }

  void spCommandListMTL::BindSampler(ezSharedPtr<spSamplerMTL> pSampler, ezUInt32 uiSet, ezUInt32 uiSlot, ezBitflags<spShaderStage> eStages)
  {
    pSampler->EnsureResourceCreated();
    const MTL::SamplerState* pMTLSamplerState = pSampler->GetSamplerState()->GetMTLSamplerState();

    if (eStages.IsSet(spShaderStage::ComputeShader))
    {
      MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::ComputeShader)];
      EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind sampler state to compute kernel. The argument buffer encoder is invalid.");

      pArgumentEncoder->setSamplerState(pMTLSamplerState, uiSlot);
    }
    else
    {
      if (eStages.IsSet(spShaderStage::VertexShader))
      {
        MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::VertexShader)];
        EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind sampler state to vertex kernel. The argument buffer encoder is invalid.");

        pArgumentEncoder->setSamplerState(pMTLSamplerState, uiSlot);
      }

      if (eStages.IsSet(spShaderStage::PixelShader))
      {
        MTL::ArgumentEncoder* pArgumentEncoder = m_ArgumentEncoders[GetResourceSetKey(uiSet, spShaderStage::PixelShader)];
        EZ_ASSERT_DEV(pArgumentEncoder != nullptr, "Cannot bind sampler state to fragment kernel. The argument buffer encoder is invalid.");

        pArgumentEncoder->setSamplerState(pMTLSamplerState, uiSlot);
      }
    }
  }

  ezSharedPtr<spBufferMTL> spCommandListMTL::GetFreeStagingBuffer(ezUInt32 uiSize)
  {
    {
      EZ_LOCK(m_SubmittedCommandsLock);

      for (auto buffer : m_AvailableStagingBuffers)
      {
        if (buffer->GetSize() < uiSize)
          continue;

        m_AvailableStagingBuffers.RemoveAndCopy(buffer);
        return buffer;
      }
    }

    const auto pBuffer = m_pDevice->GetResourceFactory()->CreateBuffer(spBufferDescription(uiSize, spBufferUsage::Staging));
    return pBuffer.Downcast<spBufferMTL>();
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_CommandList);
