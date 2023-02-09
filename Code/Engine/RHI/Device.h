#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Memory/StagingMemoryPool.h>

#include <RHI/Buffer.h>
#include <RHI/Resource.h>
#include <RHI/Swapchain.h>
#include <RHI/Texture.h>

struct spDeviceCapabilities
{
  /// \brief Specifies whether the depth target is in the range [0; 1]
  /// instead of [-1; 1]
  bool m_bIsDepthRangeZeroToOne{false};

  /// \brief Specifies whether the sampled texture origin is at the upper left corner.
  bool m_bIsUvOriginTopLeft{false};

  /// \brief Specifies whether the clip space is inverted over the Y axis.
  bool m_bIsClipSpaceYInverted{false};

  /// \brief Specifies whether the device supports compute shaders.
  bool m_bComputeShader{false};

  /// \brief Specifies whether the device supports geometry shaders.
  bool m_bGeometryShader{false};

  /// \brief Specifies whether the device supports tessellation shaders.
  bool m_bTessellationShader{false};

  /// \brief Specifies whether the device supports multiple viewports.
  bool m_bMultipleViewport{false};

  /// \brief Specifies whether the device supports LOD bias in samplers.
  bool m_bSamplerLodBias{false};

  /// \brief Specifies whether the device supports draw base vertex operations.
  bool m_bDrawBaseVertex{false};

  /// \brief Specifies whether the device supports draw base instance operations.
  bool m_bDrawBaseInstance{false};

  /// \brief Specifies whether the device supports draw indirect operations.
  bool m_bDrawIndirect{false};

  /// \brief Specifies whether the device supports draw indirect with base instance operations.
  bool m_bDrawIndirectBaseInstance{false};

  /// \brief Specifies whether the device supports wireframe fill mode.
  bool m_bFillModeWireframe{false};

  /// \brief Specifies whether the device supports anisotropic filtering.
  bool m_bSamplerAnisotropy{false};

  /// \brief Specifies whether the device can disable depth clipping.
  bool m_bDepthClipDisable{false};

  /// \brief Specifies whether the device supports one-dimensional texturing.
  bool m_bTexture1D{false};

  /// \brief Specifies whether the device supports independent blend operations.
  bool m_bIndependentBlend{false};

  /// \brief Specifies whether the device supports structured buffers.
  bool m_bStructuredBuffers{false};

  /// \brief Specifies whether the device supports creation of texture views from a subset of a texture.
  bool m_bSubsetTextureView{false};

  /// \brief Specifies whether the device supports debug markers in command lists.
  bool m_bCommandListDebugMarkers{false};

  /// \brief Specifies whether the device supports binding of buffer ranges.
  bool m_bBufferRangeBinding{false};

  /// \brief Specifies whether the device supports float64 types in shaders.
  bool m_bShaderFloat64{false};

  /// \brief Specifies whether the device supports 16 bits precision in shaders.
  bool m_bShader16BitsPrecision{false};

  /// \brief Specifies whether the device supports conservative rasterization.
  bool m_bConservaiveRasterization{false};

  /// \brief Specifies whether the device supports command lists.
  bool m_bSupportCommandLists{false};

  /// \brief Specifies whether the device supports command lists.
  bool m_bSupportConcurrentResources{false};
};

/// \brief Describes how to create a device.
struct spDeviceDescription : public ezHashableStruct<spDeviceDescription>
{
  /// \brief Indicates whether the device should support debug features.
  bool m_bDebug{false};

  /// \brief The initial width of the device.
  ezUInt32 m_uiWidth;

  /// \brief The initial height of the device.
  ezUInt32 m_uiHeight;

  /// \brief Indicates that the device will include a "main" \see spSwapchain. If this
  /// value is true, then the \a m_MainSwapchainDescription field must be set.
  bool m_bHasMainSwapchain{false};

  /// \brief The description of the main swapchain to associate to the device at the moment
  /// of creation.
  spSwapchainDescription m_MainSwapchainDescription;

  /// \brief Indicates that the "main" \see spSwapchain of the device will include a depth buffer.
  bool m_bMainSwapchainHasDepth{false};

  /// \brief An optional \see spPixelFormat for the depth buffer of the swapchain. This value must be set
  /// if \see m_bMainSwapchainHasDepth is true.
  ezEnum<spPixelFormat> m_eSwapchainDepthPixelFormat;

  /// \brief Specifies if the main swapchain will be synchronized with the window system's vertical refresh rate.
  bool m_bSyncV{false};

  /// \brief Specifies that the depth buffer of the main swapchain will be created with a range of [0; 1]. This is not the
  /// default value on OpenGL, and setting this value is not available on all platforms.
  bool m_bPreferDepthRangeZeroToOne{true};

  /// \brief Specifies that the main swapchain clip space Y direction should be normalized to bottom-to-top increasing. This
  /// is not the default value on Vulkan, and setting this value is not available on all platforms.
  bool m_bPreferStandardClipSpaceYDirection{true};

  /// \brief Specifies that the main swapchain format should be the srgb counterpart if the given one is not already a srgb format.
  bool m_bUsSrgbFormat{false};
};

/// \brief Wrapper around the graphics device. Implementations should override this
/// class and provide platform-specific functionality.
class SP_RHI_DLL spDevice : public ezReflectedClass
{
public:
  struct HardwareInfo
  {
    ezStringUtf8 m_sName;
    ezStringUtf8 m_sVendor;
    ezUInt32 m_uiID{0};
    ezUInt64 m_uiVideoMemory{0};
  };

  ~spDevice() override = default;

  /// \brief Gets the hardware info.
  EZ_NODISCARD virtual HardwareInfo GetHardwareInfo() const = 0;

  /// \brief Gets the currently used graphics API.
  EZ_NODISCARD virtual ezEnum<spGraphicsApi> GetAPI() const = 0;

  /// \brief Gets the version of the currently used graphics API.
  EZ_NODISCARD virtual spGraphicsApiVersion GetAPIVersion() const { return m_ApiVersion; }

  /// \brief Gets the resource factory associated with this graphics device.
  EZ_NODISCARD virtual spDeviceResourceFactory* GetResourceFactory() const = 0;

  /// \brief Gets the resource manager associated with this graphics device.
  EZ_NODISCARD EZ_ALWAYS_INLINE spDeviceResourceManager* GetResourceManager() const { return m_pResourceManager; }

  /// \brief Gets the texture/sampler manager associated with this graphics device.
  EZ_NODISCARD virtual spTextureSamplerManager* GetTextureSamplerManager() const = 0;

  /// \brief Gets the minimum required alignment in bytes for constant buffer offsets.
  /// \note m_uiOffset must be a multiple of this value when used for constant buffers.
  EZ_NODISCARD virtual ezUInt32 GetConstantBufferMinOffsetAlignment() const = 0;

  /// \brief Gets the minimum required alignment in bytes for structured buffer offsets.
  /// \note m_uiOffset must be a multiple of this value when used for structured buffers.
  EZ_NODISCARD virtual ezUInt32 GetStructuredBufferMinOffsetAlignment() const = 0;

  /// \brief Gets the main swapchain used as the default swapchain for new \see spGraphicsDeviceContext
  /// create without a swapchain.
  EZ_NODISCARD virtual spResourceHandle GetMainSwapchain() const = 0;

  /// \brief Gets the capabilities of this graphics device.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual const spDeviceCapabilities& GetCapabilities() const { return m_Capabilities; }

  /// \brief Gets the staging memory pool used by this graphics device.
  EZ_NODISCARD EZ_ALWAYS_INLINE spStagingMemoryPool* GetStagingMemoryPool() const { return m_pStagingMemoryPool; }

  /// \brief Submits the given \see spCommandList for execution by this device.
  /// \param [in] hCommandList The handle to the command list to execute.
  ///
  /// \note Commands submitted with this method will block the calling thread until
  /// all commands are executed.
  void SubmitCommandList(const spResourceHandle& hCommandList);

  /// \brief Submits the given \see spCommandList for execution by this device.
  /// \param [in] hCommandList The handle to the command list to execute.
  ///
  /// \note Commands submitted with this method may not be completed when this method returns.
  void SubmitCommandListAsync(const spResourceHandle& hCommandList);

  /// \brief Submits the given \see spCommandList for execution by this device.
  /// \param [in] hCommandList The handle to the command list to execute.
  /// \param [in] hFence The handle to the \see spFence which will be signaled after this submission fully completes execution.
  ///
  /// \note Commands submitted with this method will block the calling thread until
  /// all commands are executed.
  virtual void SubmitCommandList(const spResourceHandle& hCommandList, const spResourceHandle& hFence) = 0;

  /// \brief Submits the given \see spCommandList for execution by this device.
  /// \param [in] hCommandList The handle to the command list to execute.
  /// \param [in] hFence The handle to the \see spFence which will be signaled after this submission fully completes execution.
  ///
  /// \note Commands submitted with this method may not be completed when this method returns.
  virtual void SubmitCommandListAsync(const spResourceHandle& hCommandList, const spResourceHandle& hFence) = 0;

  /// \brief Blocks the calling thread until the given \see spFence is signaled.
  /// \param [in] hFence The handle to the \see spFence to wait for.
  void WaitForFence(const spResourceHandle& hFence);

  /// \brief Blocks the calling thread until the given \see spFence is signaled.
  /// \param [in] hFence The handle to the \see spFence to wait for.
  /// \param [in] timeout A timeout to wait for the \see spFence to be signaled.
  /// \returns \c true if the \a hFence was signaled within the timeout, and \c false if the timeout was reached.
  bool WaitForFence(const spResourceHandle& hFence, const ezTime& timeout);

  /// \brief Blocks the calling thread until the given \see spFence is signaled.
  /// \param [in] hFence The handle to the \see spFence to wait for.
  /// \param [in] uiNanosecondsTimeout A timeout in nano seconds to wait for the \see spFence.
  /// \returns \c true if the \a hFence was signaled within the timeout, and \c false if the timeout was reached.
  virtual bool WaitForFence(const spResourceHandle& hFence, double uiNanosecondsTimeout) = 0;

  /// \brief Blocks the calling thread until the given one or all the given \a fences are signaled.
  /// \param [in] fences The list of handles to the \see spFence instances to wait for.
  /// \param [in] bWaitAll Specifies if the method should block until all the fences has been signaled.
  void WaitForFences(const ezList<spResourceHandle>& fences, bool bWaitAll);

  /// \brief Blocks the calling thread until the given one or all the given \a fences are signaled.
  /// \param [in] fences The list of handles to the \see spFence instances to wait for.
  /// \param [in] timeout A timeout to wait for \a fences to be signaled.
  /// \param [in] bWaitAll Specifies if the method should block until all the fences has been signaled.
  /// \returns \c true when one or all the \a fences was signaled within the timeout, and \c false if the timeout was reached.
  bool WaitForFences(const ezList<spResourceHandle>& fences, bool bWaitAll, const ezTime& timeout);

  /// \brief Blocks the calling thread until the given one or all the given \a fences are signaled.
  /// \param [in] fences The list of handles to the \see spFence instances to wait for.
  /// \param [in] uiNanosecondsTimeout A timeout in nano seconds to wait \a fences.
  /// \param [in] bWaitAll Specifies if the method should block until all the fences has been signaled.
  /// \returns \c true when one or all the \a fences was signaled within the timeout, and \c false if the timeout was reached.
  virtual bool WaitForFences(const ezList<spResourceHandle>& fences, bool bWaitAll, double uiNanosecondsTimeout) = 0;

  /// \brief Resets the given \see spFence to an unsignaled state.
  /// \param [in] hFence The handle to the \see spFence to reset.
  virtual void ResetFence(const spResourceHandle& hFence) = 0;

  /// \brief Notifies the device to performs a resize operation on the main swap chain.
  /// \param [in] uiWidth The new width of the swap chain.
  /// \param [in] uiHeight The new height of the swap chain.
  ///
  /// \note This method does not do anything when the device has been created without a swap chain.
  void ResizeSwapchain(ezUInt32 uiWidth, ezUInt32 uiHeight);

  /// \brief A blocking method that returns when all submitted \see spCommandList have fully completed.
  void WaitForIdle();

  /// \brief Gets the maximum samples count supported by the \a format.
  /// \param [in] eFormat The format to get the maximum samples count for.
  /// \param [in] bIsDepthFormat Specifies whether the \a format is used for a depth buffer.
  /// \returns A \see spTextureSampleCount value representing the maximum sample count that a \see spTexture
  /// of this \a format can be created with.
  virtual ezEnum<spTextureSampleCount> GetTextureSampleCountLimit(const ezEnum<spPixelFormat>& eFormat, bool bIsDepthFormat) = 0;

  /// \brief Maps a \see spBuffer or a \see spTexture into a CPU-accessible data region.
  /// \param [in] hResource A handle to the buffer or texture resource to map.
  /// \param [in] eAccess The \see spMapAccess to use.
  /// \param [in] uiSubResource The subresource to map. Subresources are indexed first by mip level, then by array layer. For
  /// buffer resources, this parameter should be \c 0.
  const spMappedResource& Map(const spResourceHandle& hResource, const ezEnum<spMapAccess>& eAccess, ezUInt32 uiSubResource);

  /// \brief Maps a \see spBuffer or a \see spTexture into a CPU-accessible data region, and returns
  /// a structured view over that region.
  /// \param [in] hResource A handle to the buffer or texture resource to map.
  /// \param [in] eAccess The \see spMapAccess to use.
  /// \param [in] uiSubResource The subresource to map. Subresources are indexed first by mip level, then by array layer. For
  /// buffer resources, this parameter should be \c 0.
  template <typename T>
  EZ_ALWAYS_INLINE const spMappedResource& Map(const spResourceHandle& hResource, ezEnum<spMapAccess> eAccess, ezUInt32 uiSubResource)
  {
    spMappedResource mappedResource = Map(hResource, eAccess, uiSubResource);
    return spMappedResourceView<T>(mappedResource);
  }

  /// \brief Invalidates a previously mapped data region for the specified resource.
  /// \param [in] hResource A handle to the buffer or texture resource to unmap.
  /// \param [in] uiSubResource The subresource to unmap. Subresources are indexed first by mip level, then by array layer.
  /// For buffer resources, this parameter should be \c 0
  void UnMap(const spResourceHandle& hResource, ezUInt32 uiSubResource);

  /// \brief Updates the \see spTexture data with the given \a source.
  /// \param hResource The handle to the \see spTexture to update.
  /// \param pData The data source to upload to the texture.
  /// \param uiSize The size in bytes of data to upload in the texture.
  /// \param uiX The minimum X value of the updated region.
  /// \param uiY The minimum Y value of the updated region.
  /// \param uiZ The minimum Z value of the updated region.
  /// \param uiWidth The width of the updated region, in texels.
  /// \param uiHeight The height of the updated region, in texels.
  /// \param uiDepth The depth of the updated region, in texels.
  /// \param uiMipLevel The mipmap level to update. Must be less than the total number of
  /// mipmap contained in the \see spTexture.
  /// \param uiArrayLayer The array layer to update. Must be less than the total array layer
  /// count contained in the \see spTexture.
  virtual void UpdateTexture(const spResourceHandle& hResource, const void* pData, ezUInt32 uiSize, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiZ, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer) = 0;

  /// \brief Updates the \see spTexture data with the given \a source.
  /// \param hResource The handle to the \see spTexture to update.
  /// \param data The data source to upload to the texture.
  /// \param uiX The minimum X value of the updated region.
  /// \param uiY The minimum Y value of the updated region.
  /// \param uiZ The minimum Z value of the updated region.
  /// \param uiWidth The width of the updated region, in texels.
  /// \param uiHeight The height of the updated region, in texels.
  /// \param uiDepth The depth of the updated region, in texels.
  /// \param uiMipLevel The mipmap level to update. Must be less than the total number of
  /// mipmap contained in the \see spTexture.
  /// \param uiArrayLayer The array layer to update. Must be less than the total array layer
  /// count contained in the \see spTexture.
  template <typename T>
  EZ_ALWAYS_INLINE void UpdateTexture(const spResourceHandle& hResource, ezArrayPtr<T> data, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiZ, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer)
  {
    ezUInt32 uiSize = data.GetCount() * sizeof(T);
    UpdateTexture(hResource, static_cast<void*>(data.GetPtr()), uiSize, uiX, uiY, uiZ, uiWidth, uiHeight, uiDepth, uiMipLevel, uiArrayLayer);
  }

  /// \brief Updates the \see spBuffer region with new data.
  /// \param hResource The handle to the \see spBuffer to update.
  /// \param uiOffset The offset in bytes from the beginning of the buffer's storage, at which the data will be uploaded.
  /// \param pSource A pointer to the start of the data to be uploaded.
  /// \param uiSize The size in bytes of the data to be uploaded.
  void UpdateBuffer(const spResourceHandle& hResource, ezUInt32 uiOffset, const void* pSource, ezUInt32 uiSize);

  /// \brief Updates the \see spBuffer region with new data.
  /// \param hResource The handle to the \see spBuffer to update.
  /// \param uiOffset The offset in bytes from the beginning of the buffer's storage, at which the data will be uploaded.
  /// \param source The data to upload in the buffer.
  template <typename T>
  void UpdateBuffer(const spResourceHandle& hResource, ezUInt32 uiOffset, const T& source)
  {
    UpdateBuffer(hResource, uiOffset, static_cast<void*>(&source), sizeof(T));
  }

  /// \brief Updates the \see spBuffer region with new data.
  /// \param hResource The handle to the \see spBuffer to update.
  /// \param uiOffset The offset in bytes from the beginning of the buffer's storage, at which the data will be uploaded.
  /// \param source The data array to upload in the buffer.
  /// \param uiCount The number of elements in the array to upload in the buffer.
  template <typename T>
  void UpdateBuffer(const spResourceHandle& hResource, ezUInt32 uiOffset, ezArrayPtr<T> source, ezUInt32 uiCount)
  {
    UpdateBuffer(hResource, uiOffset, static_cast<void*>(source.GetPtr()), uiCount * sizeof(T));
  }

  /// \brief Copy a texture content from the \a hSource to the \a hDestination.
  /// \param hSource The resource handle of the source texture.
  /// \param hDestination The resource handle of the destination texture.
  virtual void ResolveTexture(const spResourceHandle& hSource, const spResourceHandle& hDestination) = 0;

  /// \brief Destroys the device and release all resources
  virtual void Destroy() = 0;

  /// \brief Returns whether the device has debugging enabled.
  EZ_NODISCARD virtual bool IsDebugEnabled() const = 0;

protected:
  /// \brief Constructs a new instance of the \see spDevice class.
  /// \param description The resource manager for the device.
  explicit spDevice(spDeviceDescription description)
    : m_Description(std::move(description))
  {
  }

  virtual void WaitForIdleInternal() = 0;
  virtual const spMappedResource& MapInternal(spBuffer* pBuffer, ezEnum<spMapAccess> eAccess) = 0;
  virtual const spMappedResource& MapInternal(spTexture* pTexture, ezEnum<spMapAccess> eAccess, ezUInt32 uiSubresource) = 0;
  virtual void UnMapInternal(spBuffer* pBuffer) = 0;
  virtual void UnMapInternal(spTexture* pTexture, ezUInt32 uiSubresource) = 0;
  virtual void UpdateBufferInternal(spBuffer* pBuffer, ezUInt32 uiOffset, const void* pData, ezUInt32 uiSize) = 0;

  spDeviceDescription m_Description;
  spGraphicsApiVersion m_ApiVersion;
  spDeviceCapabilities m_Capabilities;

  spDeviceResourceManager* m_pResourceManager{nullptr};
  spStagingMemoryPool* m_pStagingMemoryPool{nullptr};

private:
  spMappedResource m_InvalidDefaultMappedResource{};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spDevice);
