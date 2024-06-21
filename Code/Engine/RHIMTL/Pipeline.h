#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Pipeline.h>

namespace RHI
{
  class spDeviceMTL;
  class spResourceLayoutMTL;

  class SP_RHIMTL_DLL spComputePipelineMTL final : public spComputePipeline
  {
    EZ_ADD_DYNAMIC_REFLECTION(spComputePipelineMTL, spComputePipeline);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spComputePipelineMTL

  public:
    spComputePipelineMTL(spDeviceMTL* pDevice, const spComputePipelineDescription& description);
    ~spComputePipelineMTL() override;

    [[nodiscard]] EZ_ALWAYS_INLINE MTL::ComputePipelineState* GetPipelineState() const { return m_pPipelineState; }
    [[nodiscard]] EZ_ALWAYS_INLINE MTL::Size GetDispatchSize() const { return m_DispatchSize; }

  private:
    MTL::Device* m_mMTLDevice{nullptr};

    MTL::ComputePipelineState* m_pPipelineState{nullptr};
    MTL::Size m_DispatchSize{1, 1, 1};

    ezList<MTL::Function*> m_SpecializedFunctions;
  };

  class SP_RHIMTL_DLL spGraphicPipelineMTL final : public spGraphicPipeline
  {
    EZ_ADD_DYNAMIC_REFLECTION(spGraphicPipelineMTL, spGraphicPipeline);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spGraphicPipelineMTL

  public:
    spGraphicPipelineMTL(spDeviceMTL* pDevice, const spGraphicPipelineDescription& description);
    ~spGraphicPipelineMTL() override;

    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetNonVertexBufferCount() const { return m_uiNonVertexBufferCount; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetVertexBufferCount() const { return m_uiVertexBufferCount; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetStencilReference() const { return m_uiStencilReference; }

    [[nodiscard]] EZ_ALWAYS_INLINE bool IsScissorTestEnabled() const { return m_bScissorTestEnabled; }
    [[nodiscard]] EZ_ALWAYS_INLINE bool HasStencil() const { return m_bHasStencil; }

    [[nodiscard]] EZ_ALWAYS_INLINE MTL::RenderPipelineState* GetPipelineState() const { return m_pPipelineState; }
    [[nodiscard]] EZ_ALWAYS_INLINE MTL::DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState; }

    [[nodiscard]] EZ_ALWAYS_INLINE MTL::PrimitiveType GetPrimitiveType() const { return m_PrimitiveType; }
    [[nodiscard]] EZ_ALWAYS_INLINE MTL::CullMode GetCullMode() const { return m_CullMode; }
    [[nodiscard]] EZ_ALWAYS_INLINE MTL::Winding GetWinding() const { return m_Winding; }
    [[nodiscard]] EZ_ALWAYS_INLINE MTL::TriangleFillMode GetFillMode() const { return m_FillMode; }
    [[nodiscard]] EZ_ALWAYS_INLINE MTL::DepthClipMode GetDepthClipMode() const { return m_DepthClipMode; }
    [[nodiscard]] EZ_ALWAYS_INLINE float GetDepthBias() const { return m_fDepthBias; }
    [[nodiscard]] EZ_ALWAYS_INLINE float GetDepthBiasClamp() const { return m_fDepthBiasClamp; }
    [[nodiscard]] EZ_ALWAYS_INLINE float GetSlopeScaledDepthBias() const { return m_fSlopeScaledDepthBias; }

    [[nodiscard]] EZ_ALWAYS_INLINE ezColor GetBlendColor() const { return m_BlendColor; }

  private:
    ezDynamicArray<ezSharedPtr<spInputLayout>> m_InputLayouts;

    MTL::Device* m_pMTLDevice{nullptr};

    MTL::PrimitiveType m_PrimitiveType{MTL::PrimitiveTypeTriangle};
    MTL::CullMode m_CullMode{MTL::CullModeBack};
    MTL::Winding m_Winding{MTL::WindingCounterClockwise};
    MTL::TriangleFillMode m_FillMode{MTL::TriangleFillModeFill};
    MTL::DepthClipMode m_DepthClipMode{MTL::DepthClipModeClip};
    float m_fDepthBias{0.0f};
    float m_fDepthBiasClamp{0.0f};
    float m_fSlopeScaledDepthBias{0.0f};

    MTL::RenderPipelineState* m_pPipelineState{nullptr};
    MTL::DepthStencilState* m_pDepthStencilState{nullptr};

    ezUInt32 m_uiNonVertexBufferCount{0};
    ezUInt32 m_uiVertexBufferCount{0};
    ezUInt32 m_uiStencilReference{0};

    ezColor m_BlendColor{ezColor::Black};

    bool m_bScissorTestEnabled{false};
    bool m_bHasStencil{false};

    ezList<MTL::Function*> m_SpecializedFunctions;
  };
} // namespace RHI
