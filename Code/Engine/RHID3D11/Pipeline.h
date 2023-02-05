#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Pipeline.h>

class spDeviceD3D11;
class spResourceLayoutD3D11;

class SP_RHID3D11_DLL spComputePipelineD3D11 final : public spComputePipeline
{
  // spDeviceResource

public:
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spComputePipelineD3D11

public:
  spComputePipelineD3D11(spDeviceD3D11* pDevice, const spComputePipelineDescription& description);

private:
  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11ComputeShader* m_pComputeShader{nullptr};

  ezDynamicArray<spResourceLayoutD3D11*> m_ResourceLayouts;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spComputePipelineD3D11);

class SP_RHID3D11_DLL spGraphicPipelineD3D11 final : public spGraphicPipeline
{
  // spDeviceResource

public:
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spComputePipelineD3D11

public:
  spGraphicPipelineD3D11(spDeviceD3D11* pDevice, const spGraphicPipelineDescription& description);

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
  ezUInt32 m_StencilRef{0};

  ID3D11RasterizerState* m_pRasterizerState{nullptr};
  D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology{D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST};

  ID3D11InputLayout* m_pInputLayout{nullptr};
  ezDynamicArray<spInputLayout*> m_InputLayouts;

  ezDynamicArray<spResourceLayoutD3D11*> m_ResourceLayouts;
  ezDynamicArray<ezUInt32> m_VertexStrides;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spGraphicPipelineD3D11);
