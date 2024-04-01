#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Device.h>

#include <RHIMTL/Core.h>

namespace RHI
{
  class spDeviceResourceManagerMTL;
  class spDeviceResourceFactoryMTL;
  class spTextureSamplerManagerMTL;
  class spSwapchainMTL;
  class spBufferMTL;
  class spFenceMTL;
  class spShaderMTL;
  class spCommandListMTL;

  class SP_RHIMTL_DLL spDeviceMTL final : public spDevice
  {
    EZ_DECLARE_SINGLETON_OF_INTERFACE(spDeviceMTL, spDevice);

    // spDevice

  public:
    EZ_NODISCARD HardwareInfo GetHardwareInfo() const override;
    EZ_NODISCARD ezEnum<spGraphicsApi> GetAPI() const override;
    EZ_NODISCARD spDeviceResourceFactory* GetResourceFactory() const override;
    EZ_NODISCARD spTextureSamplerManager* GetTextureSamplerManager() const override;
    EZ_NODISCARD ezUInt32 GetConstantBufferMinOffsetAlignment() const override;
    EZ_NODISCARD ezUInt32 GetStructuredBufferMinOffsetAlignment() const override;
    EZ_NODISCARD ezSharedPtr<spSwapchain> GetMainSwapchain() const override;
    EZ_NODISCARD const spDeviceCapabilities& GetCapabilities() const override;
    void SubmitCommandList(ezSharedPtr<spCommandList> pCommandList, ezSharedPtr<spFence> pFence) override;
    bool WaitForFence(ezSharedPtr<spFence> pFence) override;
    bool WaitForFence(ezSharedPtr<spFence> pFence, double uiNanosecondsTimeout) override;
    bool WaitForFences(const ezList<ezSharedPtr<spFence>>& fences, bool bWaitAll) override;
    bool WaitForFences(const ezList<ezSharedPtr<spFence>>& fences, bool bWaitAll, double uiNanosecondsTimeout) override;
    void RaiseFence(ezSharedPtr<spFence> pFence) override;
    void ResetFence(ezSharedPtr<spFence> pFence) override;
    void Present() override;
    ezEnum<spTextureSampleCount> GetTextureSampleCountLimit(const ezEnum<spPixelFormat>& eFormat, bool bIsDepthFormat) override;
    void UpdateTexture(ezSharedPtr<spTexture> pTexture, const void* pData, ezUInt32 uiSize, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiZ, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer) override;
    virtual void UpdateIndexedIndirectBuffer(ezSharedPtr<spBuffer> pBuffer, const ezArrayPtr<spDrawIndexedIndirectCommand>& source) override;
    virtual void UpdateIndirectBuffer(ezSharedPtr<spBuffer> pBuffer, const ezArrayPtr<spDrawIndirectCommand>& source) override;
    virtual ezUInt32 GetIndexedIndirectCommandSize() override;
    virtual ezUInt32 GetIndirectCommandSize() override;
    void ResolveTexture(ezSharedPtr<spTexture> pSource, ezSharedPtr<spTexture> pDestination) override;
    void Destroy() override;
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsDebugEnabled() const override { return m_bIsDebugEnabled; }
    void BeginFrame() override;

  protected:
    void WaitForIdleInternal() override;
    const spMappedResource& MapInternal(ezSharedPtr<spBuffer> pBuffer, ezEnum<spMapAccess> eAccess) override;
    const spMappedResource& MapInternal(ezSharedPtr<spTexture> pTexture, ezEnum<spMapAccess> eAccess, ezUInt32 uiSubresource) override;
    void UnMapInternal(ezSharedPtr<spBuffer> pBuffer) override;
    void UnMapInternal(ezSharedPtr<spTexture> pTexture, ezUInt32 uiSubresource) override;
    void UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pData, ezUInt32 uiSize) override;

    // spDeviceMTL

  public:
    spDeviceMTL(ezAllocator* pAllocator, const spDeviceDescription& deviceDescription);
    ~spDeviceMTL() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::Device* GetMTLDevice() const { return m_pMTLDevice; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spMTLSupportedFeatureSet& GetSupportedFeatures() const { return m_SupportedFeatures; }

    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::CommandQueue* GetCommandQueue() const { return m_pCommandQueue; }

    MTL::ComputePipelineState* GetUnalignedBufferCopyComputePipelineState();

  private:
    void OnCommandBufferCompleted(MTL::CommandBuffer* pCommandBuffer);

    MTL::Device* m_pMTLDevice{nullptr};
    ezString m_DeviceName;

    spDeviceResourceFactoryMTL* m_pResourceFactory{nullptr};
    spTextureSamplerManagerMTL* m_pTextureSamplerManager{nullptr};

    ezSharedPtr<spSwapchainMTL> m_pMainSwapchain{nullptr};

    MTL::CommandQueue* m_pCommandQueue{nullptr};
    MTL::HandlerFunction m_CompletionHandler{nullptr};

    bool m_bIsDebugEnabled{false};
    ezArrayMap<ezUInt8, bool> m_SupportedSampleCounts;

    spMTLSupportedFeatureSet m_SupportedFeatures;

    ezSharedPtr<spShaderMTL> m_pUnalignedBufferCopyShader{nullptr};
    MTL::ComputePipelineState* m_pUnalignedBufferCopyPipelineState{nullptr};
    ezMutex m_UnalignedBufferCopyPipelineStateMutex;

    MTL::CommandBuffer* m_pLastSubmittedCommandBuffer{nullptr};

    ezMutex m_SubmittedCommandsMutex;
    ezMap<MTL::CommandBuffer*, ezSharedPtr<spCommandListMTL>> m_SubmittedCommands;

    ezMutex m_MappedResourcesMutex;
    ezMap<spMappedResourceCacheKey, spMappedResource> m_MappedResourcesCache;
  };
} // namespace RHI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHIMTL_DLL, RHI::spDeviceMTL);
