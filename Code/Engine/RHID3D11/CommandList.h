#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/CommandList.h>

#include <Foundation/Containers/List.h>

class spBufferD3D11;
class spDeviceD3D11;
class spComputePipelineD3D11;
class spFramebufferD3D11;
class spGraphicPipelineD3D11;
class spBufferRangeD3D11;
class spSamplerD3D11;
class spTextureD3D11;
class spTextureViewD3D11;
class spSwapchainD3D11;

class spCommandListD3D11 final : public spCommandList
{
  friend class spDeviceD3D11;

  // spDeviceResource

public:
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spCommandList

public:
  void Begin() override;
  void Dispatch(ezUInt32 uiGroupCountX, ezUInt32 uiGroupCountY, ezUInt32 uiGroupCountZ) override;
  void SetComputePipeline(spResourceHandle hComputePipeline) override;
  void SetGraphicPipeline(spResourceHandle hGraphicPipeline) override;
  void SetScissorRect(ezUInt32 uiSlot, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiWidth, ezUInt32 uiHeight) override;
  void SetViewport(ezUInt32 uiSlot, const spViewport& viewport) override;
  void PushDebugGroup(const ezString& sName) override;
  void PopDebugGroup() override;
  void InsertDebugMarker(const ezString& sName) override;
  void End() override;

protected:
  void ClearColorTargetInternal(ezUInt32 uiIndex, ezColor clearColor) override;
  void ClearDepthStencilTargetInternal(float fClearDepth, ezUInt8 uiClearStencil) override;
  void DrawInternal(ezUInt32 uiVertexCount, ezUInt32 uiInstanceCount, ezUInt32 uiVertexStart, ezUInt32 uiInstanceStart) override;
  void DrawIndexedInternal(ezUInt32 uiIndexCount, ezUInt32 uiInstanceCount, ezUInt32 uiIndexStart, ezUInt32 uiVertexOffset, ezUInt32 uiInstanceStart) override;
  void DrawIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride) override;
  void DrawIndexedIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset, ezUInt32 uiDrawCount, ezUInt32 uiStride) override;
  void DispatchIndirectInternal(ezSharedPtr<spBuffer> pIndirectBuffer, ezUInt32 uiOffset) override;
  void ResolveTextureInternal(ezSharedPtr<spTexture> pSource, ezSharedPtr<spTexture> pDestination) override;
  void SetFramebufferInternal(ezSharedPtr<spFramebuffer> pFramebuffer) override;
  void SetIndexBufferInternal(ezSharedPtr<spBuffer> pIndexBuffer, ezEnum<spIndexFormat> eFormat, ezUInt32 uiOffset) override;
  void SetComputeResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets) override;
  void SetGraphicResourceSetInternal(ezUInt32 uiSlot, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets) override;
  void SetVertexBufferInternal(ezUInt32 uiSlot, ezSharedPtr<spBuffer> pVertexBuffer, ezUInt32 uiOffset) override;
  void UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize) override;
  void CopyBufferInternal(ezSharedPtr<spBuffer> pSourceBuffer, ezUInt32 uiSourceOffset, ezSharedPtr<spBuffer> pDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize) override;
  void CopyTextureInternal(ezSharedPtr<spTexture> pSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, ezSharedPtr<spTexture> pDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount) override;
  void GenerateMipmapsInternal(ezSharedPtr<spTexture> pTexture) override;


  // spCommandListD3D11

public:
  spCommandListD3D11(spDeviceD3D11* pDeviceD3D11, const spCommandListDescription& description);
  ~spCommandListD3D11() override;

  void Reset();

  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11CommandList* GetD3D11CommandList() const { return m_pCommandList; }

private:
  static constexpr ezUInt32 s_uiMaxCachedConstantBuffers = 16;
  static constexpr ezUInt32 s_uiMaxCachedTextureViews = 16;
  static constexpr ezUInt32 s_uiMaxCachedSamplerStates = 4;
  static constexpr ezUInt32 s_uiMaxCachedUnorderedAccessViews = 8;

  struct BoundTextureInfo
  {
    ezUInt32 m_uiSlot;
    ezBitflags<spShaderStage> m_eStages;
    ezUInt32 m_uiResourceSet;
  };

  void ClearState();
  void ResetManagedState();
  void ClearSets(ezDynamicArray<spCommandListResourceSet>& sets);
  void ActivateResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet, bool bCompute);
  ezSharedPtr<spBufferD3D11> GetFreeStagingBuffer(ezUInt32 uiSize);
  ezUInt32 GetConstantBufferBase(ezUInt32 uiSlot, bool bCompute) const;
  ezUInt32 GetUnorderedAccessViewBase(ezUInt32 uiSlot, bool bCompute) const;
  ezUInt32 GetTextureBase(ezUInt32 uiSlot, bool bCompute) const;
  ezUInt32 GetSamplerBase(ezUInt32 uiSlot, bool bCompute) const;
  ezSharedPtr<spBufferRangeD3D11> GetBufferRange(ezSharedPtr<spShaderResource> pResource, ezUInt32 uiOffset) const;
  void BindConstantBuffer(ezSharedPtr<spBufferRangeD3D11> pBufferRange, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages);
  void BindStorageBufferView(ezSharedPtr<spBufferRangeD3D11> pBufferRange, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages);
  void BindTextureView(ezSharedPtr<spTextureViewD3D11> pTextureView, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages, ezUInt32 uiSetSlot);
  void BindUnorderedAccessView(ezSharedPtr<spTextureD3D11> pTexture, ezSharedPtr<spBufferD3D11> pBuffer, ID3D11UnorderedAccessView* pUAV, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages, ezUInt32 uiSetSlot);
  void BindSampler(ezSharedPtr<spSamplerD3D11> pSampler, ezUInt32 uiSlot, const ezBitflags<spShaderStage>& eStages);
  void UnbindSRVTexture(const spTextureViewDescription& desc);
  void UnbindUAVTexture(const spTextureViewDescription& desc);
  void UnbindUAVBuffer(ezSharedPtr<spBufferD3D11> pBuffer);
  void UnbindUAVBufferPipeline(ezSharedPtr<spBufferD3D11> pBuffer, bool bCompute);
  void PreDraw();
  void PreDispatch();
  void FlushViewports();
  void FlushScissorRects();
  void FlushVertexBindings();
  void PackRangeParams(ezSharedPtr<spBufferRangeD3D11> pBufferRange);
  void TrackBoundUAVBuffer(ezSharedPtr<spBufferD3D11> pBuffer, ezUInt32 uiSlot, bool bCompute);
  void OnComplete();

  ID3D11CommandList* m_pCommandList{nullptr};
  ID3D11DeviceContext* m_pDeviceContext{nullptr};
  ID3D11DeviceContext1* m_pDeviceContext1{nullptr};
  ID3DUserDefinedAnnotation* m_pUserDefinedAnnotation{nullptr};

  bool m_bHasStarted{false};

  spFramebufferD3D11* m_pFramebuffer{nullptr};

  bool m_bViewportsChanged{false};
  ezDynamicArray<D3D11_VIEWPORT> m_Viewports;

  bool m_bScissorRectsChanged{false};
  ezDynamicArray<D3D11_RECT> m_ScissorRects;

  ezUInt32 m_uiNumVertexBuffers{0};
  ezDynamicArray<ID3D11Buffer*> m_VertexBuffers;
  ezDynamicArray<ezUInt32> m_VertexStrides;
  ezDynamicArray<ezUInt32> m_VertexOffsets;

  // --- Cached Pipeline State ---

  ezSharedPtr<spBufferD3D11> m_pIndexBuffer{nullptr};
  ezUInt32 m_uiIndexBufferOffset{0};

  ID3D11BlendState* m_pBlendState{nullptr};
  ezColor m_BlendFactor;

  ID3D11DepthStencilState* m_pDepthStencilState{nullptr};
  ezUInt32 m_uiStencilRef{0};

  ID3D11RasterizerState* m_pRasterizerState{nullptr};
  D3D11_PRIMITIVE_TOPOLOGY m_ePrimitiveTopology{D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED};

  ID3D11InputLayout* m_pInputLayout{nullptr};
  ID3D11VertexShader* m_pVertexShader{nullptr};
  ID3D11PixelShader* m_pPixelShader{nullptr};
  ID3D11GeometryShader* m_pGeometryShader{nullptr};
  ID3D11HullShader* m_pHullShader{nullptr};
  ID3D11DomainShader* m_pDomainShader{nullptr};
  ID3D11ComputeShader* m_pComputeShader{nullptr};

  ezDynamicArray<spCommandListResourceSet> m_GraphicResourceSets;
  ezDynamicArray<bool> m_InvalidatedGraphicResourceSets;

  ezDynamicArray<spCommandListResourceSet> m_ComputeResourceSets;
  ezDynamicArray<bool> m_InvalidatedComputeResourceSets;

  bool m_bIsVertexBindingsDirty{false};
  ezSmallArray<ID3D11Buffer*, 1> m_ConstantBuffersOut;
  ezSmallArray<ezUInt32, 1> m_FirstConstantBufferRef;
  ezSmallArray<ezUInt32, 1> m_ConstantBuffersRefCounts;

  // --- Cached Resources ---

  ezStaticArray<ezSharedPtr<spBufferRangeD3D11>, s_uiMaxCachedConstantBuffers> m_CachedVertexConstantBuffers;
  ezStaticArray<ezSharedPtr<spBufferRangeD3D11>, s_uiMaxCachedConstantBuffers> m_CachedPixelConstantBuffers;

  ezStaticArray<ezSharedPtr<spTextureViewD3D11>, s_uiMaxCachedTextureViews> m_CachedVertexTextureViews;
  ezStaticArray<ezSharedPtr<spTextureViewD3D11>, s_uiMaxCachedTextureViews> m_CachedPixelTextureViews;

  ezStaticArray<ezSharedPtr<spSamplerD3D11>, s_uiMaxCachedSamplerStates> m_CachedVertexSamplerStates;
  ezStaticArray<ezSharedPtr<spSamplerD3D11>, s_uiMaxCachedSamplerStates> m_CachedPixelSamplerStates;

  ezArrayMap<spTextureViewDescription, BoundTextureInfo> m_BoundSRVs;
  ezArrayMap<spTextureViewDescription, BoundTextureInfo> m_BoundUAVs;

  ezStaticArray<std::pair<ezSharedPtr<spBufferD3D11>, ezInt32>, s_uiMaxCachedUnorderedAccessViews> m_CachedComputeUAVBuffers;
  ezStaticArray<std::pair<ezSharedPtr<spBufferD3D11>, ezInt32>, s_uiMaxCachedUnorderedAccessViews> m_CachedGraphicUAVBuffers;

  ezList<ezSharedPtr<spBufferD3D11>> m_FreeBuffers;
  ezList<ezSharedPtr<spBufferD3D11>> m_SubmittedStagingBuffers;

  ezList<ezSharedPtr<spSwapchainD3D11>> m_ReferencedSwapchains;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spCommandListD3D11);
