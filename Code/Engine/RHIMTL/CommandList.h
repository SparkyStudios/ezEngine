#pragma once

#include "ResourceSet.h"


#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/CommandList.h>

#include <Foundation/Containers/List.h>

namespace RHI
{
  class spBufferMTL;
  class spDeviceMTL;
  class spComputePipelineMTL;
  class spFramebufferMTL;
  class spFramebufferMTLBase;
  class spGraphicPipelineMTL;
  class spBufferRangeMTL;
  class spSamplerMTL;
  class spTextureMTL;
  class spTextureViewMTL;
  class spSwapchainMTL;
  class spScopeProfilerMTL;
  class spShaderProgramMTL;

  class SP_RHIMTL_DLL spCommandListMTL final : public spCommandList
  {
    friend class spDeviceMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spCommandListMTL, spCommandList);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spCommandList

  public:
    void Begin() override;
    void Dispatch(ezUInt32 uiGroupCountX, ezUInt32 uiGroupCountY, ezUInt32 uiGroupCountZ) override;
    void SetComputePipeline(ezSharedPtr<spComputePipeline> pComputePipeline) override;
    void SetGraphicPipeline(ezSharedPtr<spGraphicPipeline> pGraphicPipeline) override;
    void SetScissorRect(ezUInt32 uiSlot, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiWidth, ezUInt32 uiHeight) override;
    void SetViewport(ezUInt32 uiSlot, const spViewport& viewport) override;
    void PushProfileScope(ezStringView sName) override;
    void PopProfileScope(ezSharedPtr<spScopeProfiler>& scopeProfiler) override;
    void PushDebugGroup(ezStringView sName) override;
    void PopDebugGroup() override;
    void InsertDebugMarker(ezStringView sName) override;
    void End() override;
    void Reset() override;

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
    void PushConstantsInternal(ezBitflags<spShaderStage> eStage, const void* pData, ezUInt32 uiOffset, ezUInt32 uiSize) override;
    void UpdateBufferInternal(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pSourceData, ezUInt32 uiSize) override;
    void CopyBufferInternal(ezSharedPtr<spBuffer> pSourceBuffer, ezUInt32 uiSourceOffset, ezSharedPtr<spBuffer> pDestBuffer, ezUInt32 uiDestOffset, ezUInt32 uiSize) override;
    void CopyTextureInternal(ezSharedPtr<spTexture> pSourceTexture, ezUInt32 uiSourceX, ezUInt32 uiSourceY, ezUInt32 uiSourceZ, ezUInt32 uiSourceMipLevel, ezUInt32 uiSourceBaseArrayLayer, ezSharedPtr<spTexture> pDestinationTexture, ezUInt32 uiDestX, ezUInt32 uiDestY, ezUInt32 uiDestZ, ezUInt32 uiDestMipLevel, ezUInt32 uiDestBaseArrayLayer, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiLayerCount) override;
    void GenerateMipmapsInternal(ezSharedPtr<spTexture> pTexture) override;

    // spCommandListMTL

  public:
    spCommandListMTL(spDeviceMTL* pDeviceMTL, const spCommandListDescription& description);
    ~spCommandListMTL() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::CommandBuffer* GetMTLCommandBuffer() const { return m_pCommandBuffer; }

    MTL::CommandBuffer* Commit();

    void SetCompletionFence(ezSharedPtr<spFenceMTL> pFence);
    void OnCompleted(MTL::CommandBuffer* pCommandBuffer);

  private:
    struct BoundResource
    {
      spResourceHandle m_hResource;
      ezUInt32 m_uiSlot;
      ezUInt32 m_uiOffset;
      spShaderStage::Enum m_eStage;

      EZ_NODISCARD EZ_ALWAYS_INLINE bool Check(spShaderStage::Enum eStage, spResourceHandle hResource) const { return m_hResource == hResource && m_eStage == eStage; }

      EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const BoundResource& other) const
      {
        return Check(other.m_eStage, other.m_hResource) && m_uiSlot == other.m_uiSlot && m_uiOffset == other.m_uiOffset;
      }
    };

    static bool IsResourceSetEqual(spCommandListResourceSet& set, ezSharedPtr<spResourceSet> pResourceSet, ezUInt32 uiDynamicOffsetCount, const ezUInt32* pDynamicOffsets);
    EZ_NODISCARD EZ_ALWAYS_INLINE static bool GetResourceSetKey(ezUInt32 uiSet, spShaderStage::Enum eStage) { return ezHashingUtils::CombineHashValues32(uiSet, eStage); }
    EZ_NODISCARD EZ_ALWAYS_INLINE static bool GetResourceSetSlotKey(ezUInt32 uiSet, spShaderStage::Enum eStage, ezUInt32 uiSlot) { return ezHashingUtils::CombineHashValues32(ezHashingUtils::CombineHashValues32(uiSet, eStage), uiSlot); }

    bool PreDraw();
    void PreDispatch();

    void EnsureNoRenderPass();
    void EndCurrentRenderPass();

    void EnsureNoBlitEncoder();

    void EnsureNoComputeEncoder();

    bool EnsureRenderPass();
    void EnsureBlitEncoder();
    void EnsureComputeEncoder();

    bool HasAnyUnsetClearValues();

    bool BeginCurrentRenderPass();

    void FlushViewports();
    void FlushScissorRects();

    void ClearBoundResources();

    void EnsureArgumentBuffer(ezUInt32 uiSlot, ezSharedPtr<spShaderProgramMTL> pProgram, ezSharedPtr<spResourceSetMTL> pResourceSet, ezEnum<spShaderStage> eStage);
    void BindArgumentBuffer(ezUInt32 uiSlot, ezEnum<spShaderStage> eStage);

    void ActivateGraphicResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet);
    void ActivateComputeResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet);
    void ActivateResourceSet(ezUInt32 uiSlot, const spCommandListResourceSet& resourceSet);

    void BindBuffer(ezSharedPtr<spBufferRangeMTL> pBuffer, ezUInt32 uiSet, ezUInt32 uiSlot, ezBitflags<spShaderStage> eStages);
    void BindTexture(ezSharedPtr<spTextureViewMTL> pTextureView, ezUInt32 uiSet, ezUInt32 uiSlot, ezBitflags<spShaderStage> eStages);
    void BindSampler(ezSharedPtr<spSamplerMTL> pSampler, ezUInt32 uiSet, ezUInt32 uiSlot, ezBitflags<spShaderStage> eStages);

    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsRenderCommandEncoderActive() const { return m_pRenderCommandEncoder != nullptr; }
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsBlitCommandEncoderActive() const { return m_pBlitCommandEncoder != nullptr; }
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsComputeCommandEncoderActive() const { return m_pComputeCommandEncoder != nullptr; }

    ezSharedPtr<spBufferMTL> GetFreeStagingBuffer(ezUInt32 uiSize);

    MTL::Device* m_pMTLDevice{nullptr};

    MTL::CommandBuffer* m_pCommandBuffer{nullptr};

    bool m_bHasStarted{false};

    // ezSharedPtr<spScopeProfilerMTL> m_pCurrentScopeProfiler{nullptr};
    bool m_bIsInDebugGroup{false};
    ezStringView m_sDebugGroupName;

    ezSharedPtr<spFramebufferMTLBase> m_pFramebuffer{nullptr};
    bool m_bCurrentFramebufferEverActive{false};

    bool m_bViewportsChanged{false};
    ezDynamicArray<MTL::Viewport> m_Viewports;

    bool m_bScissorRectsChanged{false};
    ezDynamicArray<MTL::ScissorRect> m_ScissorRects;

    // --- Command Encoders ---

    MTL::RenderCommandEncoder* m_pRenderCommandEncoder{nullptr};
    MTL::BlitCommandEncoder* m_pBlitCommandEncoder{nullptr};
    MTL::ComputeCommandEncoder* m_pComputeCommandEncoder{nullptr};

    // --- Cached Pipeline State ---

    ezArrayMap<ezUInt32, BoundResource> m_BoundResources;

    ezArrayMap<ezUInt32, MTL::ArgumentEncoder*> m_ArgumentEncoders;
    ezArrayMap<ezUInt32, ezSharedPtr<spBufferMTL>> m_ArgumentBuffers;

    ezSharedPtr<spBufferMTL> m_pIndexBuffer{nullptr};
    ezUInt32 m_uiIndexBufferOffset{0};
    MTL::IndexType m_eIndexType{MTL::IndexTypeUInt16};

    ezDynamicArray<ezColor> m_ClearColors;
    float m_fClearDepth{ezMath::NaN<float>()};
    ezUInt8 m_uiClearStencil{0};

    ezDynamicArray<ezSharedPtr<spBufferMTL>> m_VertexBuffers;
    ezUInt32 m_uiNumVertexBuffers{0};
    ezDynamicArray<ezUInt32> m_VertexOffsets;
    ezDynamicArray<bool> m_ActiveVertexBuffers;

    spCommandListPushConstant m_PushConstant;

    ezDynamicArray<spCommandListResourceSet> m_GraphicResourceSets;
    ezDynamicArray<bool> m_ActiveGraphicResourceSets;
    bool m_bGraphicPipelineChanged{false};

    ezDynamicArray<spCommandListResourceSet> m_ComputeResourceSets;
    ezDynamicArray<bool> m_ActiveComputeResourceSets;
    bool m_bComputePipelineChanged{false};

    ezDynamicArray<ezSharedPtr<spBufferMTL>> m_AvailableStagingBuffers;
    ezMap<MTL::CommandBuffer*, ezDynamicArray<ezSharedPtr<spBufferMTL>>> m_SubmittedStagingBuffers;
    ezMutex m_SubmittedCommandsLock;
    ezSharedPtr<spFenceMTL> m_CompletionFence{nullptr};
  };
} // namespace RHI
