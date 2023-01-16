#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

class SP_RHI_DLL spCommandList : public spDeviceResource
{
public:
  virtual void Begin() = 0;
  virtual void ClearColorTarget(ezUInt32 uiIndex, ezColor clearColor) = 0;
  virtual void ClearDepthStencilTarget(float fClearDepth, ezUInt8 uiClearStencil = 0) = 0;
  virtual void Draw(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart) = 0;
  virtual void DrawIndexed(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart) = 0;
  virtual void DrawIndirect(spResourceHandle hIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride) = 0;
  virtual void DrawIndexedIndirect(spResourceHandle hIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride) = 0;
  virtual void Dispatch(ezUInt32 uiGroupCountX, ezUInt32 uiGroupCountY, ezUInt32 uiGroupCountZ) = 0;
  virtual void DispatchIndirect(spResourceHandle hIndirectBuffer, ezUInt32 uiOffset) = 0;
  virtual void ResolveTexture(spResourceHandle hSource, spResourceHandle hDestination) = 0;
  virtual void SetFramebuffer(spResourceHandle hFramebuffer) = 0;
  virtual void SetIndexBuffer(spResourceHandle hIndexBuffer, ezEnum<spIndexFormat> eFormat) = 0;
  virtual void SetIndexBuffer(spResourceHandle hIndexBuffer, ezEnum<spIndexFormat> eFormat, ezUInt32 uiOffset) = 0;
  virtual void SetComputePipeline(spResourceHandle hComputePipeline) = 0;
  virtual void SetGraphicsPipeline(spResourceHandle hGraphicsPipeline) = 0;
  virtual void SetComputeResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet) = 0;
  virtual void SetComputeResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezDynamicArray<ezUInt32> dynamicOffsets) = 0;
  virtual void SetComputeResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezUInt32 uiDynamicOffsetCount, ezUInt32* pDynamicOffsets) = 0;
  virtual void SetGraphicsResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet) = 0;
  virtual void SetGraphicsResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezDynamicArray<ezUInt32> dynamicOffsets) = 0;
  virtual void SetGraphicsResourceSet(ezUInt32 uiSlot, spResourceHandle hResourceSet, ezUInt32 uiDynamicOffsetCount, ezUInt32* pDynamicOffsets) = 0;
  virtual void SetScissorRect(ezUInt32 uiSlot, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiWidth, ezUInt32 uiHeight) = 0;
  virtual void SetFullScissorRect() = 0;
  virtual void SetFullScissorRect(ezUInt32 uiSlot) = 0;
  virtual void SetVertexBuffer(ezUInt32 uiSlot, spResourceHandle hVertexBuffer) = 0;
  virtual void SetVertexBuffer(ezUInt32 uiSlot, spResourceHandle hVertexBuffer, ezUInt32 uiOffset) = 0;
  virtual void SetViewport(ezUInt32 uiSlot, const spViewport& viewport) = 0;
  virtual void SetFullViewport() = 0;
  virtual void SetFullViewport(ezUInt32 uiSlot) = 0;
  virtual void UpdateBuffer(spResourceHandle hBuffer, ezUInt32 uiOffset, void* pSourceData, ezUInt32 uiSize) = 0;
  template <typename T>
  void UpdateBuffer(spResourceHandle hBuffer, ezUInt32 uiOffset, const T& source);
  template <typename T>
  void UpdateBuffer(spResourceHandle hBuffer, ezUInt32 uiOffset, const T* pSource, ezUInt32 uiSize);
  template <typename T>
  void UpdateBuffer(spResourceHandle hBuffer, ezUInt32 uiOffset, ezArrayPtr<T> source, ezUInt32 uiCount);
  virtual void CopyBuffer(spResourceHandle hSourceBuffer, ezUInt32 uiSourceOffset, spResourceHandle hDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize) = 0;
  virtual void CopyTexture(spResourceHandle hSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, spResourceHandle hDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiFormat, ezUInt32 uiLayerCount) = 0;
  virtual void GenerateMipmaps(spResourceHandle hTexture) = 0;
  virtual void PushDebugGroup(const ezString& sName) = 0;
  virtual void PopDebugGroup() = 0;
  virtual void InsertDebugMarker(const ezString& sName) = 0;
  virtual void End() = 0;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spCommandList);
