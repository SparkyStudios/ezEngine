#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

class spBuffer;
class spComputePipeline;
class spFramebuffer;
class spGraphicPipeline;
class spResourceSet;
class spTexture;
class spScopeProfiler;

/// \brief Data structure holding draw arrays commands for indirect drawing.
struct SP_RHI_DLL alignas(16) spDrawIndirectCommand
{
  EZ_DECLARE_POD_TYPE();

  /// \brief The number of indices (in an instance) to be drawn.
  ezUInt32 m_uiCount;

  /// \brief The number of instances to be drawn.
  ezUInt32 m_uiInstanceCount;

  /// \brief The start index in the enabled arrays.
  ezUInt32 m_uiFirstIndex;

  /// \brief The base instance for use in fetching instanced vertex attributes.
  ezUInt32 m_uiBaseInstance;
};

struct SP_RHI_DLL alignas(16) spDrawIndexedIndirectCommand
{
  EZ_DECLARE_POD_TYPE();

  /// \brief The number of elements (in an instance) to be drawn.
  ezUInt32 m_uiCount;

  /// \brief The number of instances to be drawn.
  ezUInt32 m_uiInstanceCount;

  /// \brief Index of the first element to be drawn.
  ezUInt32 m_uiFirstIndex;

  /// \brief Constant that should be added to each element when choosing elements the enabled vertex arrays.
  ezUInt32 m_uiBaseVertex;

  /// \brief The base instance for use in fetching instanced vertex attributes.
  ezUInt32 m_uiBaseInstance;
};

struct SP_RHI_DLL spCommandListIndexBuffer : public ezHashableStruct<spCommandListIndexBuffer>
{
  spCommandListIndexBuffer() = default;
  spCommandListIndexBuffer(spResourceHandle hIndexBuffer, const ezEnum<spIndexFormat>& eFormat, ezUInt32 uiOffset);

  /// \brief Compares this \see spCommandListIndexBuffer to an \a other instance for equality.
  EZ_ALWAYS_INLINE bool operator==(const spCommandListIndexBuffer& other) const
  {
    return m_hIndexBuffer == other.m_hIndexBuffer && m_eIndexFormat == other.m_eIndexFormat && m_uiOffset == other.m_uiOffset;
  }

  /// \brief Compares this \see spCommandListIndexBuffer to an \a other instance for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spCommandListIndexBuffer& other) const
  {
    return !(*this == other);
  }

  spResourceHandle m_hIndexBuffer;
  ezEnum<spIndexFormat> m_eIndexFormat;
  ezUInt32 m_uiOffset{0};
};

struct SP_RHI_DLL spCommandListResourceSet : public ezHashableStruct<spCommandListResourceSet>
{
  spCommandListResourceSet() = default;
  spCommandListResourceSet(spResourceHandle hResource, ezUInt32 uiOffsetCount, const ezUInt32* pOffsets);
  ~spCommandListResourceSet();

  /// \brief Compares this \see spCommandListResourceSet to an \a other instance for equality.
  EZ_ALWAYS_INLINE bool operator==(const spCommandListResourceSet& other) const
  {
    return m_hResourceSet == other.m_hResourceSet && m_Offsets == other.m_Offsets;
  }

  /// \brief Compares this \see spCommandListResourceSet to an \a other instance for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spCommandListResourceSet& other) const
  {
    return !(*this == other);
  }

  spResourceHandle m_hResourceSet;
  ezArrayPtr<ezUInt32> m_Offsets;
};

struct SP_RHI_DLL spCommandListDescription : public ezHashableStruct<spCommandListDescription>
{
};

/// \brief A device resource which allows the recording of graphics commands, which can later be executed by a
/// \a spDevice. Before graphics commands can be issued, the \a spCommandList::Begin() method must be invoked.
/// When the \a spCommandList is ready to be executed, \a spCommandList::End() must be invoked, and then
/// \a spDevice::SubmitCommandList(spCommandList) should be used.
///
/// \note \a spCommandList instances cannot be executed multiple times per-recording. When executed by a
/// \a spDevice, they must be reset and commands must be issued again.
class SP_RHI_DLL spCommandList : public spDeviceResource
{
  friend class spDeviceResourceManager;

public:
  virtual void Begin() = 0;

  void ClearColorTarget(ezUInt32 uiIndex, ezColor clearColor);

  void ClearDepthStencilTarget(float fClearDepth);

  void ClearDepthStencilTarget(float fClearDepth, ezUInt8 uiClearStencil);

  void Draw(ezUInt32 uiVertexCount);

  void Draw(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart);

  void DrawIndexed(ezUInt32 uiIndexCount);

  void DrawIndexed(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart);

  void DrawIndirect(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride);

  void DrawIndexedIndirect(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride);

  virtual void Dispatch(ezUInt32 uiGroupCountX, ezUInt32 uiGroupCountY, ezUInt32 uiGroupCountZ) = 0;

  void DispatchIndirect(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset);

  void ResolveTexture(ezSharedPtr<spTexture> pSource, ezSharedPtr<spTexture> pDestination);

  void SetFramebuffer(ezSharedPtr<spFramebuffer> pFramebuffer);

  void SetIndexBuffer(ezSharedPtr<spBuffer> pIndexBuffer, const ezEnum<spIndexFormat>& eFormat);

  void SetIndexBuffer(ezSharedPtr<spBuffer> pIndexBuffer, const ezEnum<spIndexFormat>& eFormat, ezUInt32 uiOffset);

  virtual void SetComputePipeline(ezSharedPtr<spComputePipeline> pComputePipeline) = 0;

  virtual void SetGraphicPipeline(ezSharedPtr<spGraphicPipeline> pGraphicPipeline) = 0;

  void SetComputeResourceSet(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet);

  void SetComputeResourceSet(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezArrayPtr<ezUInt32> dynamicOffsets);

  void SetComputeResourceSet(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets);

  void SetGraphicResourceSet(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet);

  void SetGraphicResourceSet(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezArrayPtr<ezUInt32> dynamicOffsets);

  void SetGraphicResourceSet(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets);

  void SetVertexBuffer(ezUInt32 uiSlot, ezSharedPtr<spBuffer> pVertexBuffer);

  void SetVertexBuffer(ezUInt32 uiSlot, ezSharedPtr<spBuffer> pVertexBuffer, ezUInt32 uiOffset);

  virtual void SetScissorRect(ezUInt32 uiSlot, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiWidth, ezUInt32 uiHeight) = 0;

  void SetFullScissorRect();

  void SetFullScissorRect(ezUInt32 uiSlot);

  virtual void SetViewport(ezUInt32 uiSlot, const spViewport& viewport) = 0;

  void SetFullViewport();

  void SetFullViewport(ezUInt32 uiSlot);

  void UpdateBuffer(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize);

  template <typename T>
  void UpdateBuffer(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const T& source)
  {
    UpdateBuffer(pBuffer, uiOffset, &source, 1);
  }

  template <typename T>
  void UpdateBuffer(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const T* pSource, ezUInt32 uiCount)
  {
    UpdateBuffer(pBuffer, uiOffset, reinterpret_cast<const void*>(pSource), uiCount * sizeof(T));
  }

  template <typename T>
  void UpdateBuffer(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, ezArrayPtr<T> source, ezUInt32 uiCount)
  {
    UpdateBuffer(pBuffer, uiOffset, reinterpret_cast<const void*>(source.GetPtr()), uiCount * sizeof(T));
  }

  void CopyBuffer(ezSharedPtr<spBuffer> pSourceBuffer, ezUInt32 uiSourceOffset, ezSharedPtr<spBuffer> pDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize);

  void CopyTexture(ezSharedPtr<spTexture> pSourceTexture, ezSharedPtr<spTexture> pDestinationTexture);

  void CopyTexture(ezSharedPtr<spTexture> pSourceTexture, ezSharedPtr<spTexture> pDestinationTexture, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer);

  void CopyTexture(ezSharedPtr<spTexture> pSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, ezSharedPtr<spTexture> pDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount);

  void GenerateMipmaps(ezSharedPtr<spTexture> pTexture);

  virtual void PushProfileScope(ezStringView sName) = 0;

  virtual void PopProfileScope(ezSharedPtr<spScopeProfiler>& scopeProfiler) = 0;

  virtual void PushDebugGroup(ezStringView sName) = 0;

  virtual void PopDebugGroup() = 0;

  virtual void InsertDebugMarker(ezStringView sName) = 0;

  virtual void End() = 0;

protected:
  spCommandList(spCommandListDescription description);

  virtual void ClearColorTargetInternal(ezUInt32 uiIndex, ezColor clearColor) = 0;
  virtual void ClearDepthStencilTargetInternal(float fClearDepth, ezUInt8 uiClearStencil) = 0;
  virtual void DrawInternal(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart) = 0;
  virtual void DrawIndexedInternal(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart) = 0;
  virtual void DrawIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride) = 0;
  virtual void DrawIndexedIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride) = 0;
  virtual void DispatchIndirectInternal(ezSharedPtr<spBuffer> hIndirectBuffer, ezUInt32 uiOffset) = 0;
  virtual void ResolveTextureInternal(ezSharedPtr<spTexture> pSource, ezSharedPtr<spTexture> pDestination) = 0;
  virtual void SetFramebufferInternal(ezSharedPtr<spFramebuffer> pFramebuffer) = 0;
  virtual void SetIndexBufferInternal(ezSharedPtr<spBuffer> pIndexBuffer, ezEnum<spIndexFormat> eFormat, ezUInt32 uiOffset) = 0;
  virtual void SetGraphicResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets) = 0;
  virtual void SetComputeResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets) = 0;
  virtual void SetVertexBufferInternal(ezUInt32 uiSlot, ezSharedPtr<spBuffer> pVertexBuffer, ezUInt32 uiOffset) = 0;
  virtual void UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize) = 0;
  virtual void CopyBufferInternal(ezSharedPtr<spBuffer> pSourceBuffer, ezUInt32 uiSourceOffset, ezSharedPtr<spBuffer> pDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize) = 0;
  virtual void CopyTextureInternal(ezSharedPtr<spTexture> pSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, ezSharedPtr<spTexture> pDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount) = 0;
  virtual void GenerateMipmapsInternal(ezSharedPtr<spTexture> pTexture) = 0;

  virtual void ClearCachedState();

  spCommandListDescription m_Description;

  ezSharedPtr<spFramebuffer> m_pFramebuffer{nullptr};
  ezSharedPtr<spGraphicPipeline> m_pGraphicPipeline{nullptr};
  ezSharedPtr<spComputePipeline> m_pComputePipeline{nullptr};

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
private:
  void IndexBufferValidation(ezUInt32 uiIndexCount) const;
  void DrawValidation() const;

  ezSharedPtr<spBuffer> m_pIndexBuffer{nullptr};
  ezEnum<spIndexFormat> m_IndexBufferFormat;
#endif
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spCommandList);
