#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Buffer.h>
#include <RHIMTL/CommandList.h>
#include <RHIMTL/ResourceFactory.h>
#include <RHIMTL/Shader.h>
#include <RHIMTL/Swapchain.h>
#include <RHIMTL/Texture.h>

#include <RHIMTL/Device.h>

// clang-format off
EZ_IMPLEMENT_SINGLETON(RHI::spDeviceMTL);

EZ_BEGIN_STATIC_REFLECTED_TYPE(RHI::spDeviceMTL, RHI::spDevice, 1, ezRTTINoAllocator)
EZ_END_STATIC_REFLECTED_TYPE;

EZ_DEFINE_AS_POD_TYPE(MTL::DrawPrimitivesIndirectArguments);
EZ_DEFINE_AS_POD_TYPE(MTL::DrawIndexedPrimitivesIndirectArguments);
// clang-format on

namespace RHI
{
  constexpr std::initializer_list<spTextureSampleCount::Enum> kTextureSampleCounts = {
    spTextureSampleCount::None,
    spTextureSampleCount::TwoSamples,
    spTextureSampleCount::FourSamples,
    spTextureSampleCount::EightSamples,
    spTextureSampleCount::SixteenSamples,
    spTextureSampleCount::ThirtyTwoSamples,
  };

  constexpr ezUInt8 kUnalignedBufferCopyShaderCode[] = R"(
#include <metal_stdlib>
using namespace metal;

struct CopyInfo
{
  uint32_t SrcOffset;
  uint32_t DstOffset;
  uint32_t CopySize;
};

kernel void copy_bytes(
  device uint8_t* src [[ buffer(0) ]],
  device uint8_t* dst [[ buffer(1) ]],
  constant CopyInfo& info [[ buffer(2) ]])
{
  for (uint32_t i = 0; i < info.CopySize; i++)
  {
    dst[i + info.DstOffset] = src[i + info.SrcOffset];
  }
})";

#pragma region spDeviceMTL

  spDevice::HardwareInfo
  spDeviceMTL::GetHardwareInfo() const
  {
    return m_HardwareInfo;
  }

  ezEnum<spGraphicsApi> spDeviceMTL::GetAPI() const
  {
    return spGraphicsApi::Metal;
  }

  spDeviceResourceFactory* spDeviceMTL::GetResourceFactory() const
  {
    return m_pResourceFactory;
  }

  spTextureSamplerManager* spDeviceMTL::GetTextureSamplerManager() const
  {
    return m_pTextureSamplerManager;
  }

  ezUInt32 spDeviceMTL::GetConstantBufferMinOffsetAlignment() const
  {
    return m_SupportedFeatures.IsMacOS() ? 16 : 256;
  }

  ezUInt32 spDeviceMTL::GetStructuredBufferMinOffsetAlignment() const
  {
    return 16;
  }

  ezSharedPtr<spSwapchain> spDeviceMTL::GetMainSwapchain() const
  {
    return m_pMainSwapchain;
  }

  const spDeviceCapabilities& spDeviceMTL::GetCapabilities() const
  {
    return m_Capabilities;
  }

  void spDeviceMTL::SubmitCommandList(ezSharedPtr<spCommandList> pCommandList, ezSharedPtr<spFence> pFence)
  {
    if (const auto pCommandListMTL = pCommandList.Downcast<spCommandListMTL>(); pCommandListMTL != nullptr)
    {
      pCommandListMTL->m_pCommandBuffer->addCompletedHandler(m_CompletionHandler);

      {
        EZ_LOCK(m_SubmittedCommandsMutex);

        if (pFence != nullptr)
          m_SubmittedCommands.Insert(pCommandListMTL->m_pCommandBuffer, pFence.Downcast<spFenceMTL>());

        m_pLastSubmittedCommandBuffer = pCommandListMTL->Commit();
      }
    }
  }

  bool spDeviceMTL::WaitForFence(ezSharedPtr<RHI::spFence> pFence)
  {
    if (pFence == nullptr)
      return false;

    return pFence.Downcast<spFenceMTL>()->Wait();
  }

  bool spDeviceMTL::WaitForFence(ezSharedPtr<RHI::spFence> pFence, double uiNanosecondsTimeout)
  {
    if (pFence == nullptr)
      return false;

    return pFence.Downcast<spFenceMTL>()->Wait(ezTime::Nanoseconds(uiNanosecondsTimeout));
  }

  bool spDeviceMTL::WaitForFences(const ezList<ezSharedPtr<RHI::spFence>>& fences, bool bWaitAll)
  {
    for (auto it = fences.GetIterator(); it.IsValid(); it.Next())
    {
      if (it->Downcast<spFenceMTL>()->Wait())
      {
        if (bWaitAll)
          continue;

        return true;
      }

      return false;
    }

    return true;
  }

  bool spDeviceMTL::WaitForFences(const ezList<ezSharedPtr<RHI::spFence>>& fences, bool bWaitAll, double uiNanosecondsTimeout)
  {
    for (auto it = fences.GetIterator(); it.IsValid(); it.Next())
    {
      if (it->Downcast<spFenceMTL>()->Wait(ezTime::Nanoseconds(uiNanosecondsTimeout)))
      {
        if (bWaitAll)
          continue;

        return true;
      }

      return false;
    }

    return true;
  }

  void spDeviceMTL::RaiseFence(ezSharedPtr<RHI::spFence> pFence)
  {
    if (pFence == nullptr)
      return;

    pFence.Downcast<spFenceMTL>()->Raise();
  }

  void spDeviceMTL::ResetFence(ezSharedPtr<RHI::spFence> pFence)
  {
    if (pFence == nullptr)
      return;

    pFence.Downcast<spFenceMTL>()->Reset();
  }

  void spDeviceMTL::Present()
  {
    if (m_pMainSwapchain != nullptr)
      m_pMainSwapchain->Present();
  }

  ezEnum<spTextureSampleCount> spDeviceMTL::GetTextureSampleCountLimit(const ezEnum<spPixelFormat>& eFormat, bool bIsDepthFormat)
  {
    for (int i = m_SupportedSampleCounts.GetCount() - 1; i >= 0; i--)
      if (m_SupportedSampleCounts.GetValue(i))
        return spTextureSampleCount::GetSampleCount(m_SupportedSampleCounts.GetKey(i));

    return spTextureSampleCount::None;
  }

  void spDeviceMTL::UpdateTexture(ezSharedPtr<spTexture> pTexture, const void* pData, ezUInt32 uiSize, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiZ, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer)
  {
    auto pTextureMTL = pTexture.Downcast<spTextureMTL>();
    pTextureMTL->EnsureResourceCreated();

    if (pTextureMTL->GetMTLStagingBuffer() == nullptr)
    {
      spTextureDescription stagingTextureDescription;
      stagingTextureDescription.m_uiWidth = uiWidth;
      stagingTextureDescription.m_uiHeight = uiHeight;
      stagingTextureDescription.m_uiDepth = uiDepth;
      stagingTextureDescription.m_uiArrayLayers = 1;
      stagingTextureDescription.m_uiMipCount = 1;
      stagingTextureDescription.m_eFormat = pTextureMTL->GetFormat();
      stagingTextureDescription.m_eUsage = spTextureUsage::Staging;
      stagingTextureDescription.m_eDimension = pTextureMTL->GetDimension();
      auto pStagingTexture = m_pResourceFactory->CreateTexture(stagingTextureDescription);

      UpdateTexture(pStagingTexture, pData, uiSize, 0, 0, 0, uiWidth, uiHeight, uiDepth, 0, 0);
      auto cl = m_pResourceFactory->CreateCommandList({});

      cl->Begin();
      cl->CopyTexture(
        pStagingTexture, 0, 0, 0, 0, 0,
        pTextureMTL, uiX, uiY, uiZ, uiMipLevel, uiArrayLayer,
        uiWidth, uiHeight, uiDepth, 1);
      cl->End();

      SubmitCommandList(cl, nullptr);

      cl.Clear();
      pStagingTexture.Clear();
    }
    else
    {
      ezUInt32 uiDstRowPitch, uiDstDepthPitch;
      pTextureMTL->GetSubresourceLayout(uiMipLevel, uiArrayLayer, uiDstRowPitch, uiDstDepthPitch);
      ezUInt32 uiDstOffset = spTextureHelper::CalculateSubresource(pTextureMTL, uiMipLevel, uiArrayLayer);

      ezUInt32 uiSrcRowPitch = spPixelFormatHelper::GetRowPitch(uiWidth, pTextureMTL->GetFormat());
      ezUInt32 uiSrcDepthPitch = spPixelFormatHelper::GetDepthPitch(uiSrcRowPitch, uiHeight, pTextureMTL->GetFormat());

      spTextureHelper::CopyTextureRegion(
        pData,
        0, 0, 0,
        uiSrcRowPitch, uiSrcDepthPitch,
        (ezUInt8*)pTextureMTL->GetMTLStagingBuffer()->contents() + uiDstOffset,
        uiX, uiY, uiZ,
        uiDstRowPitch, uiDstDepthPitch,
        uiWidth, uiHeight, uiDepth,
        pTextureMTL->GetFormat());
    }
  }

  void spDeviceMTL::UpdateIndexedIndirectBuffer(ezSharedPtr<spBuffer> pBuffer, const ezArrayPtr<spDrawIndexedIndirectCommand>& source)
  {
    ezDynamicArray<MTL::DrawIndexedPrimitivesIndirectArguments> args;
    args.SetCount(source.GetCount());

    for (ezUInt32 i = 0, l = args.GetCount(); i < l; i++) {
      auto& arg = args[i];
      const auto& command = source[i];
      arg.indexCount = command.m_uiCount;
      arg.instanceCount = command.m_uiInstanceCount;
      arg.indexStart = command.m_uiFirstIndex;
      arg.baseVertex = command.m_uiBaseVertex;
      arg.baseInstance = command.m_uiBaseInstance;
    }

    UpdateBuffer(pBuffer, 0, args.GetData(), args.GetCount());
  }

  void spDeviceMTL::UpdateIndirectBuffer(ezSharedPtr<spBuffer> pBuffer, const ezArrayPtr<spDrawIndirectCommand>& source)
  {
    ezDynamicArray<MTL::DrawPrimitivesIndirectArguments> args;
    args.SetCount(source.GetCount());

    for (ezUInt32 i = 0, l = args.GetCount(); i < l; i++) {
      auto& arg = args[i];
      const auto& command = source[i];
      arg.vertexCount = command.m_uiCount;
      arg.instanceCount = command.m_uiInstanceCount;
      arg.vertexStart = command.m_uiFirstIndex;
      arg.baseInstance = command.m_uiBaseInstance;
    }

    UpdateBuffer(pBuffer, 0, args.GetArrayPtr());
  }

  ezUInt32 spDeviceMTL::GetIndexedIndirectCommandSize()
  {
    return sizeof(MTL::DrawIndexedPrimitivesIndirectArguments);
  }

  ezUInt32 spDeviceMTL::GetIndirectCommandSize()
  {
    return sizeof(MTL::DrawPrimitivesIndirectArguments);
  }

  void spDeviceMTL::ResolveTexture(ezSharedPtr<RHI::spTexture> pSource, ezSharedPtr<RHI::spTexture> pDestination)
  {
    // TODO
  }

  void spDeviceMTL::Destroy()
  {
    WaitForIdle();

    if (m_pUnalignedBufferCopyPipelineState != nullptr)
    {
      m_pUnalignedBufferCopyShader.Clear();
      SP_RHI_MTL_RELEASE(m_pUnalignedBufferCopyPipelineState);
    }

    m_pMainSwapchain.Clear();
    SP_RHI_MTL_RELEASE(m_pCommandQueue);
    SP_RHI_MTL_RELEASE(m_pMTLDevice);
  }

  void spDeviceMTL::BeginFrame()
  {
    spDevice::BeginFrame();

    m_pMainSwapchain->GetNextDrawable();
  }

  void spDeviceMTL::WaitForIdleInternal()
  {
    MTL::CommandBuffer* pLastCommandBuffer = nullptr;

    {
      EZ_LOCK(m_SubmittedCommandsMutex);

      pLastCommandBuffer = m_pLastSubmittedCommandBuffer;
      SP_RHI_MTL_RETAIN(pLastCommandBuffer);
    }

    if (pLastCommandBuffer != nullptr && pLastCommandBuffer->status() != MTL::CommandBufferStatusCompleted)
    {
      pLastCommandBuffer->waitUntilCompleted();
    }

    SP_RHI_MTL_RELEASE(pLastCommandBuffer);
  }

  const spMappedResource& spDeviceMTL::MapInternal(ezSharedPtr<spBuffer> pBuffer, ezEnum<spMapAccess> eAccess)
  {
    const spMappedResourceCacheKey key(pBuffer, 0);
    EZ_LOCK(m_MappedResourcesMutex);

    spMappedResource mappedResource;
    if (m_MappedResourcesCache.TryGetValue(key, mappedResource))
    {
      EZ_ASSERT_DEV(mappedResource.GetAccess() == eAccess, "The given resource is already mapped in a different access mode.");
    }
    else
    {
      auto pBufferMTL = pBuffer.Downcast<spBufferMTL>();
      pBufferMTL->EnsureResourceCreated();

      void* pData = pBufferMTL->GetMTLBuffer()->contents();

      mappedResource = spMappedResource(
        pBufferMTL->GetHandle(),
        eAccess,
        pData,
        pBufferMTL->GetSize(),
        0,
        pBufferMTL->GetSize(),
        pBufferMTL->GetSize());
    }

    m_MappedResourcesCache[key] = std::move(mappedResource);
    EZ_IGNORE_UNUSED(m_MappedResourcesCache[key].AddRef());

    return m_MappedResourcesCache[key];
  }

  const spMappedResource& spDeviceMTL::MapInternal(ezSharedPtr<RHI::spTexture> pTexture, ezEnum<RHI::spMapAccess> eAccess, ezUInt32 uiSubresource)
  {
    const spMappedResourceCacheKey key(pTexture, uiSubresource);
    EZ_LOCK(m_MappedResourcesMutex);

    spMappedResource mappedResource;
    if (m_MappedResourcesCache.TryGetValue(key, mappedResource))
    {
      EZ_ASSERT_DEV(mappedResource.GetAccess() == eAccess, "The given resource is already mapped in a different access mode.");
    }
    else
    {
      auto pTextureMTL = pTexture.Downcast<spTextureMTL>();
      pTextureMTL->EnsureResourceCreated();

      EZ_ASSERT_DEV(pTextureMTL->GetMTLStagingBuffer() != nullptr, "Can't map a texture without a staging buffer!");

      ezUInt32 uiMipLevel, uiArrayLayer, uiWidth, uiHeight, uiDepth, uiRowPitch, uiDepthPitch;
      void* pData = pTextureMTL->GetMTLStagingBuffer()->contents();

      spTextureHelper::GetMipLevelAndArrayLayer(pTextureMTL, uiSubresource, uiMipLevel, uiArrayLayer);
      spTextureHelper::GetMipDimensions(pTextureMTL, uiMipLevel, uiWidth, uiHeight, uiDepth);
      ezUInt32 uiSubresourceSize = pTextureMTL->GetSubresourceSize(uiMipLevel, uiArrayLayer);
      pTextureMTL->GetSubresourceLayout(uiMipLevel, uiArrayLayer, uiRowPitch, uiDepthPitch);
      ezUInt32 uiOffset = spTextureHelper::CalculateSubresource(pTextureMTL, uiMipLevel, uiArrayLayer);
      ezUInt8* pOffsetData = (ezUInt8*)pData + uiOffset;

      mappedResource = spMappedResource(
        pTextureMTL->GetHandle(),
        eAccess,
        pOffsetData,
        uiSubresourceSize,
        uiSubresource,
        uiRowPitch,
        uiDepthPitch);
    }

    m_MappedResourcesCache[key] = std::move(mappedResource);
    EZ_IGNORE_UNUSED(m_MappedResourcesCache[key].AddRef());

    return m_MappedResourcesCache[key];
  }

  void spDeviceMTL::UnMapInternal(ezSharedPtr<RHI::spBuffer> pBuffer)
  {
    const spMappedResourceCacheKey key(pBuffer, 0);
    EZ_LOCK(m_MappedResourcesMutex);

    bool bDone = m_MappedResourcesCache.Contains(key);
    EZ_ASSERT_DEV(bDone, "The given resource is not mapped.");

    bDone = m_MappedResourcesCache.Remove(key);
    EZ_ASSERT_DEV(bDone, "Unable to unmap the resource.");
  }

  void spDeviceMTL::UnMapInternal(ezSharedPtr<RHI::spTexture> pTexture, ezUInt32 uiSubresource)
  {
    const spMappedResourceCacheKey key(pTexture, uiSubresource);
    EZ_LOCK(m_MappedResourcesMutex);

    bool bDone = m_MappedResourcesCache.Contains(key);
    EZ_ASSERT_DEV(bDone, "The given resource is not mapped.");

    bDone = m_MappedResourcesCache.Remove(key);
    EZ_ASSERT_DEV(bDone, "Unable to unmap the resource.");
  }

  void spDeviceMTL::UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pData, ezUInt32 uiSize)
  {
    auto pBufferMTL = pBuffer.Downcast<spBufferMTL>();
    pBufferMTL->EnsureResourceCreated();

    void* destPtr = pBufferMTL->GetMTLBuffer()->contents();
    ezUInt8* destOffsetPtr = (ezUInt8*)destPtr + uiOffset;
    ezMemoryUtils::RawByteCopy(destOffsetPtr, pData, uiSize);
  }

  spDeviceMTL::spDeviceMTL(ezAllocatorBase* pAllocator, const spDeviceDescription& deviceDescription)
    : spDevice(pAllocator, deviceDescription)
    , m_SingletonRegistrar(this)
    , m_pMTLDevice(MTL::CreateSystemDefaultDevice())
    , m_SupportedFeatures(m_pMTLDevice)
  {
    m_HardwareInfo.m_sName = m_pMTLDevice->name()->utf8String();
    m_HardwareInfo.m_sVendor = "Apple";
    m_HardwareInfo.m_uiID = m_pMTLDevice->registryID();

    ezUInt32 uiMajor = m_SupportedFeatures.GetMaxFeatureSet() / 10000;
    ezUInt32 uiMinor = m_SupportedFeatures.GetMaxFeatureSet() % 10000;

    m_ApiVersion = spGraphicsApiVersion(uiMajor, uiMinor, 0, 0);

    m_Capabilities.m_bComputeShader = true;
    m_Capabilities.m_bGeometryShader = false;
    m_Capabilities.m_bTessellationShader = false;
    m_Capabilities.m_bMultipleViewport = m_SupportedFeatures.IsSupported(MTL::FeatureSet_macOS_GPUFamily1_v3);
    m_Capabilities.m_bSamplerLodBias = false;
    m_Capabilities.m_bDrawBaseVertex = m_SupportedFeatures.IsDrawBaseVertexInstanceSupported();
    m_Capabilities.m_bDrawBaseInstance = m_SupportedFeatures.IsDrawBaseVertexInstanceSupported();
    m_Capabilities.m_bDrawIndirect = true;
    m_Capabilities.m_bDrawIndirectBaseInstance = true;
    m_Capabilities.m_bFillModeWireframe = true;
    m_Capabilities.m_bSamplerAnisotropy = true;
    m_Capabilities.m_bDepthClipDisable = true;
    m_Capabilities.m_bTexture1D = true; // TODO = Should be macOS 10.11+ and iOS 11.0+.
    m_Capabilities.m_bIndependentBlend = true;
    m_Capabilities.m_bStructuredBuffers = true;
    m_Capabilities.m_bSubsetTextureView = true;
    m_Capabilities.m_bCommandListDebugMarkers = true;
    m_Capabilities.m_bBufferRangeBinding = true;
    m_Capabilities.m_bShaderFloat64 = false;
    m_Capabilities.m_bIsUvOriginTopLeft = true;
    m_Capabilities.m_bIsDepthRangeZeroToOne = true;
    m_Capabilities.m_bIsClipSpaceYInverted = false;

    m_CompletionHandler = [this](MTL::CommandBuffer* pCommandBuffer) {
      OnCommandBufferCompleted(pCommandBuffer);
    };

    m_pResourceManager = EZ_DEFAULT_NEW(spDefaultDeviceResourceManager, this);
    m_pResourceFactory = EZ_DEFAULT_NEW(spDeviceResourceFactoryMTL, this);
    m_pTextureSamplerManager = EZ_DEFAULT_NEW(spTextureSamplerManagerMTL, this);

    m_pCommandQueue = m_pMTLDevice->newCommandQueue();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    spScopedMTLResource nsString(NS::String::string("RHI Metal Device Command Queue", NS::UTF8StringEncoding));
    m_pCommandQueue->setLabel(*nsString);
#endif

    for (auto eSampleCount : kTextureSampleCounts)
      m_SupportedSampleCounts.Insert(eSampleCount, m_pMTLDevice->supportsTextureSampleCount(eSampleCount));

    if (deviceDescription.m_bHasMainSwapchain)
      m_pMainSwapchain = m_pResourceFactory->CreateSwapchain(deviceDescription.m_MainSwapchainDescription).Downcast<spSwapchainMTL>();
  }

  spDeviceMTL::~spDeviceMTL()
  {
    m_pResourceManager->ReleaseResources();

    EZ_DEFAULT_DELETE(m_pResourceManager);
    EZ_DEFAULT_DELETE(m_pResourceFactory);
    EZ_DEFAULT_DELETE(m_pTextureSamplerManager);
  }

  MTL::ComputePipelineState* spDeviceMTL::GetUnalignedBufferCopyComputePipelineState()
  {
    EZ_LOCK(m_UnalignedBufferCopyPipelineStateMutex);

    if (m_pUnalignedBufferCopyPipelineState == nullptr)
    {
      EZ_ASSERT_DEV(m_pUnalignedBufferCopyShader == nullptr, "The unaligned buffer copy shader is already initialized!");

      spScopedMTLResource descriptor(MTL::ComputePipelineDescriptor::alloc()->init());
      auto* buffer0 = descriptor->buffers()->object(0);
      auto* buffer1 = descriptor->buffers()->object(1);

      buffer0->setMutability(MTL::MutabilityMutable);
      buffer1->setMutability(MTL::MutabilityMutable);

      spShaderDescription shaderDesc;
      shaderDesc.m_eShaderStage = spShaderStage::ComputeShader;
      shaderDesc.m_Buffer = ezMakeByteArrayPtr(kUnalignedBufferCopyShaderCode, static_cast<ezUInt32>(sizeof(kUnalignedBufferCopyShaderCode)));
      shaderDesc.m_sEntryPoint = ezMakeHashedString("copy_bytes");

      m_pUnalignedBufferCopyShader = m_pResourceFactory->CreateShader(shaderDesc).Downcast<spShaderMTL>();

      NS::Error* pError = nullptr;
      descriptor->setComputeFunction(m_pUnalignedBufferCopyShader->GetMTLShaderFunction());
      m_pUnalignedBufferCopyPipelineState = m_pMTLDevice->newComputePipelineState(*descriptor, MTL::PipelineOptionNone, nullptr, &pError);

      if (pError != nullptr)
      {
        ezLog::Error("Failed to create unaligned buffer copy pipeline state: {0}", pError->localizedDescription()->utf8String());
      }
    }

    return m_pUnalignedBufferCopyPipelineState;
  }

  void spDeviceMTL::OnCommandBufferCompleted(MTL::CommandBuffer* pCommandBuffer)
  {
    {
      EZ_LOCK(m_SubmittedCommandsMutex);

      ezSharedPtr<spFenceMTL> pFence(nullptr);
      if (m_SubmittedCommands.TryGetValue(pCommandBuffer, pFence))
      {
        pFence->Raise();
        m_SubmittedCommands.Remove(pCommandBuffer);
      }

      if (m_pLastSubmittedCommandBuffer == m_pCurrentCommandBuffer)
        m_pLastSubmittedCommandBuffer = nullptr;
    }

    SP_RHI_MTL_RELEASE(m_pCurrentCommandBuffer);
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Device);
