#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Pipeline.h>

namespace RHI
{
  class spDeviceD3D11;
  class spResourceLayoutD3D11;

  class SP_RHID3D11_DLL spComputePipelineD3D11 final : public spComputePipeline
  {
    EZ_ADD_DYNAMIC_REFLECTION(spComputePipelineD3D11, spComputePipeline);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spComputePipelineD3D11

  public:
    spComputePipelineD3D11(spDeviceD3D11* pDevice, const spComputePipelineDescription& description);
    ~spComputePipelineD3D11() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11ComputeShader* GetComputeShader() const { return m_pComputeShader; }

  private:
    ID3D11Device* m_pD3D11Device{nullptr};
    ID3D11ComputeShader* m_pComputeShader{nullptr};
  };

  class SP_RHID3D11_DLL spGraphicPipelineD3D11 final : public spGraphicPipeline
  {
    EZ_ADD_DYNAMIC_REFLECTION(spGraphicPipelineD3D11, spGraphicPipeline);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spComputePipelineD3D11

  public:
    spGraphicPipelineD3D11(spDeviceD3D11* pDevice, const spGraphicPipelineDescription& description);
    ~spGraphicPipelineD3D11() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11VertexShader* GetVertexShader() const { return m_pVertexShader; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11GeometryShader* GetGeometryShader() const { return m_pGeometryShader; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11HullShader* GetHullShader() const { return m_pHullShader; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11DomainShader* GetDomainShader() const { return m_pDomainShader; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11PixelShader* GetPixelShader() const { return m_pPixelShader; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11BlendState* GetBlendState() const { return m_pBlendState; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezColor GetBlendFactor() const { return m_BlendFactor; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11DepthStencilState* GetDepthStencilState() const { return m_pDepthStencilState; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetStencilRef() const { return m_uiStencilRef; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11RasterizerState* GetRasterizerState() const { return m_pRasterizerState; }
    EZ_NODISCARD EZ_ALWAYS_INLINE D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return m_PrimitiveTopology; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<ezSharedPtr<spInputLayout>>& GetInputLayouts() const { return m_InputLayouts; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<ezUInt32>& GetVertexStrides() const { return m_VertexStrides; }

  private:
    ID3D11Device* m_pD3D11Device{nullptr};

    ID3D11VertexShader* m_pVertexShader{nullptr};
    ID3D11GeometryShader* m_pGeometryShader{nullptr};
    ID3D11HullShader* m_pHullShader{nullptr};
    ID3D11DomainShader* m_pDomainShader{nullptr};
    ID3D11PixelShader* m_pPixelShader{nullptr};

    ID3D11BlendState* m_pBlendState{nullptr};
    ezColor m_BlendFactor{ezColor::Black};

    ID3D11DepthStencilState* m_pDepthStencilState{nullptr};
    ezUInt32 m_uiStencilRef{0};

    ID3D11RasterizerState* m_pRasterizerState{nullptr};
    D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology{D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST};

    ID3D11InputLayout* m_pInputLayout{nullptr};
    ezDynamicArray<ezSharedPtr<spInputLayout>> m_InputLayouts;

    ezDynamicArray<ezUInt32> m_VertexStrides;
  };
} // namespace RHI
