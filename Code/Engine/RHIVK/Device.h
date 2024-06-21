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

#pragma once

#include <RHIVK/RHIVKDLL.h>

#include <RHI/Device.h>

namespace RHI
{
  struct SP_RHIVK_DLL spDeviceDescriptionVK final : public spDeviceDescription
  {
    ezDynamicArray<ezStringView> m_InstanceExtensions;
    ezDynamicArray<ezStringView> m_DeviceExtensions;
  };

  class SP_RHIVK_DLL spDeviceVK final : public spDevice
  {
    EZ_DECLARE_SINGLETON_OF_INTERFACE(spDeviceVK, spDevice);

    // spDevice

  public:
    [[nodiscard]] HardwareInfo GetHardwareInfo() const override;
    [[nodiscard]] ezEnum<spGraphicsApi> GetAPI() const override;
    [[nodiscard]] spDeviceResourceFactory* GetResourceFactory() const override;
    [[nodiscard]] spTextureSamplerManager* GetTextureSamplerManager() const override;
    [[nodiscard]] ezUInt32 GetConstantBufferMinOffsetAlignment() const override;
    [[nodiscard]] ezUInt32 GetStructuredBufferMinOffsetAlignment() const override;
    [[nodiscard]] ezSharedPtr<spSwapchain> GetMainSwapchain() const override;
    [[nodiscard]] const spDeviceCapabilities& GetCapabilities() const override;
    void SubmitCommandList(ezSharedPtr<spCommandList> pCommandList, ezSharedPtr<spFence> pFence) override;
    bool WaitForFence(ezSharedPtr<spFence> pFence) override;
    bool WaitForFence(ezSharedPtr<spFence> pFence, double uiNanosecondsTimeout) override;
    bool WaitForFences(const ezList<ezSharedPtr<spFence>>& fences, bool bWaitAll) override;
    bool WaitForFences(const ezList<ezSharedPtr<spFence>>& fences, bool bWaitAll, double uiNanosecondsTimeout) override;
    void RaiseFence(ezSharedPtr<spFence> pFence) override;
    void ResetFence(ezSharedPtr<spFence> pFence) override;
    void Present() override;
    ezEnum<spTextureSampleCount> GetTextureSampleCountLimit(const ezEnum<spPixelFormat>& eFormat, bool bIsDepthFormat) override;
    void UpdateTexture(ezSharedPtr<spTexture> pTexture, const void* pData, ezUInt64 uiSize, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiZ, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer) override;
    void UpdateIndexedIndirectBuffer(ezSharedPtr<spBuffer> pBuffer, const ezArrayPtr<spDrawIndexedIndirectCommand>& source) override;
    void UpdateIndirectBuffer(ezSharedPtr<spBuffer> pBuffer, const ezArrayPtr<spDrawIndirectCommand>& source) override;
    ezUInt32 GetIndexedIndirectCommandSize() override;
    ezUInt32 GetIndirectCommandSize() override;
    void ResolveTexture(ezSharedPtr<spTexture> pSource, ezSharedPtr<spTexture> pDestination) override;
    void Destroy() override;
    [[nodiscard]] EZ_ALWAYS_INLINE bool IsDebugEnabled() const override { return m_bIsDebugEnabled; }
    void BeginFrame() override;
    void EndFrame() override;

  protected:
    void WaitForIdleInternal() override;
    const spMappedResource& MapInternal(ezSharedPtr<spBuffer> pBuffer, ezEnum<spMapAccess> eAccess) override;
    const spMappedResource& MapInternal(ezSharedPtr<spTexture> pTexture, ezEnum<spMapAccess> eAccess, ezUInt32 uiSubresource) override;
    void UnMapInternal(ezSharedPtr<spBuffer> pBuffer) override;
    void UnMapInternal(ezSharedPtr<spTexture> pTexture, ezUInt32 uiSubresource) override;
    void UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pData, ezUInt32 uiSize) override;

    // spDeviceVK

  public:
    spDeviceVK(ezAllocator* pAllocator, const spDeviceDescriptionVK& description);
    ~spDeviceVK() override;

  private:
    [[nodiscard]] bool HasSurfaceExtension(ezStringView sExtensionName) const;
    void EnableDebugCallback(vk::DebugReportFlagsEXT flags = vk::DebugReportFlagsEXT(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError)) const;

    void CreateInstance(const spDeviceDescriptionVK& description);
    void CreatePhysicalDevice();

    bool m_bIsDebugEnabled{false};
    vk::DebugReportCallbackEXT* m_pDebugCallback{nullptr};

    ezSet<ezStringView> m_SurfaceExtensions;
    bool m_bStandardValidationLayerAvailable{false};
    bool m_bKhronosValidationLayerAvailable{false};

    vk::Instance m_Instance{nullptr};
    vk::PhysicalDevice m_PhysicalDevice{nullptr};
    vk::PhysicalDeviceProperties m_PhysicalDeviceProperties{};
    vk::PhysicalDeviceFeatures m_PhysicalDeviceFeatures{};
    vk::PhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties{};

    PFN_vkGetPhysicalDeviceProperties2 m_vkGetPhysicalDeviceProperties2{nullptr};
  };
} // namespace RHI
