#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/ResourceManager.h>

#include <RHI/Input.h>

#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>

// clang-format off
EZ_DEFINE_AS_POD_TYPE(D3D11_INPUT_ELEMENT_DESC);
// clang-format on

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spDeviceResourceManagerD3D11, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  static LPCSTR GetSemanticName(const ezEnum<spInputElementLocationSemantic>& eSemantic)
  {
    switch (eSemantic)
    {
      case spInputElementLocationSemantic::Position:
        return "POSITION";
      case spInputElementLocationSemantic::Normal:
        return "NORMAL";
      case spInputElementLocationSemantic::Tangent:
        return "TANGENT";
      case spInputElementLocationSemantic::BiTangent:
        return "BITANGENT";
      case spInputElementLocationSemantic::TexCoord:
        return "TEXCOORD";
      case spInputElementLocationSemantic::Color:
        return "COLOR";
      case spInputElementLocationSemantic::BoneWeights:
        return "BONEWEIGHTS";
      case spInputElementLocationSemantic::BoneIndices:
        return "BONEINDICES";
      default:
        return "UNKNOWN";
    }
  }

  spDeviceResourceManagerD3D11::spDeviceResourceManagerD3D11(spDeviceD3D11* pDevice)
    : spDefaultDeviceResourceManager(pDevice)
  {
    m_pD3D11Device = pDevice->GetD3D11Device();
  }

  spDeviceResourceManagerD3D11::~spDeviceResourceManagerD3D11()
  {
    for (auto& value : m_BlendStates)
      SP_RHI_DX11_RELEASE(value.value);

    for (auto& value : m_DepthStencilStates)
      SP_RHI_DX11_RELEASE(value.value);

    for (auto& value : m_RasterizerStates)
      SP_RHI_DX11_RELEASE(value.value);

    for (auto& value : m_InputLayouts)
      SP_RHI_DX11_RELEASE(value.value);

    m_BlendStates.Clear();
    m_DepthStencilStates.Clear();
    m_RasterizerStates.Clear();
    m_InputLayouts.Clear();
  }

  void spDeviceResourceManagerD3D11::GetPipelineResources(const spBlendState& blendState, const spDepthState& depthState, const spStencilState& stencilState, const spRasterizerState& rasterizerState, bool bMultisample, ezArrayPtr<spInputLayoutDescription> inputLayouts, ezByteArrayPtr vertexShaderByteCode, ID3D11BlendState*& pBlendState, ID3D11DepthStencilState*& pDepthStencilState, ID3D11RasterizerState*& pRasterizerState, ID3D11InputLayout*& pInputLayouts)
  {
    EZ_LOCK(m_Mutex);

    pBlendState = GetBlendState(blendState);
    pDepthStencilState = GetDepthStencilState(depthState, stencilState);
    pRasterizerState = GetRasterizerState(rasterizerState, bMultisample);
    pInputLayouts = GetInputLayout(inputLayouts, vertexShaderByteCode);
  }


  ID3D11BlendState* spDeviceResourceManagerD3D11::GetBlendState(const spBlendState& blendState)
  {
    EZ_ASSERT_DEV(m_Mutex.IsLocked(), "Invalid call to GetBlendState. The mutex must be locked first!");

    ID3D11BlendState* pBlendState = nullptr;
    const ezUInt32 i = m_BlendStates.Find(blendState.CalculateHash());

    if (i == ezInvalidIndex)
    {
      pBlendState = CreateBlendState(blendState);
      m_BlendStates.Insert(blendState.CalculateHash(), pBlendState);
    }

    return pBlendState;
  }

  ID3D11DepthStencilState* spDeviceResourceManagerD3D11::GetDepthStencilState(const spDepthState& depthState, const spStencilState& stencilState)
  {
    EZ_ASSERT_DEV(m_Mutex.IsLocked(), "Invalid call to GetDepthStencilState. The mutex must be locked first!");

    ID3D11DepthStencilState* pDepthStencilState = nullptr;
    const ezUInt32 i = m_DepthStencilStates.Find(depthState.CalculateHash());

    if (i == ezInvalidIndex)
    {
      pDepthStencilState = CreateDepthStencilState(depthState, stencilState);
      m_DepthStencilStates.Insert(depthState.CalculateHash(), pDepthStencilState);
    }

    return pDepthStencilState;
  }

  ID3D11RasterizerState* spDeviceResourceManagerD3D11::GetRasterizerState(const spRasterizerState& rasterizerState, bool bMultisample)
  {
    EZ_ASSERT_DEV(m_Mutex.IsLocked(), "Invalid call to GetRasterizerState. The mutex must be locked first!");

    ID3D11RasterizerState* pRasterizerState = nullptr;
    RasterizerStateCacheKey key{rasterizerState.CalculateHash(), bMultisample};
    const ezUInt32 i = m_RasterizerStates.Find(key);

    if (i == ezInvalidIndex)
    {
      pRasterizerState = CreateRasterizerState(rasterizerState, bMultisample);
      m_RasterizerStates.Insert(key, pRasterizerState);
    }

    return pRasterizerState;
  }

  ID3D11InputLayout* spDeviceResourceManagerD3D11::GetInputLayout(const ezArrayPtr<spInputLayoutDescription>& inputLayouts, const ezByteArrayPtr& vertexShaderByteCode)
  {
    EZ_ASSERT_DEV(m_Mutex.IsLocked(), "Invalid call to GetInputLayout. The mutex must be locked first!");

    if (vertexShaderByteCode.IsEmpty() || inputLayouts.IsEmpty())
      return nullptr;

    ID3D11InputLayout* pInputLayout = nullptr;
    const InputLayoutCacheKey key = InputLayoutCacheKey::GetTemporaryKey(inputLayouts);
    const ezUInt32 i = m_InputLayouts.Find(key);

    if (i == ezInvalidIndex)
    {
      pInputLayout = CreateInputLayout(inputLayouts, vertexShaderByteCode);
      m_InputLayouts.Insert(InputLayoutCacheKey::GetPermanentKey(inputLayouts), pInputLayout);
    }

    return pInputLayout;
  }

  ID3D11BlendState* spDeviceResourceManagerD3D11::CreateBlendState(const spBlendState& blendState) const
  {
    D3D11_BLEND_DESC desc;
    ezUInt32 i = 0;

    for (const auto& attachment : blendState.m_AttachmentStates)
    {
      desc.RenderTarget[i].BlendEnable = attachment.m_bEnabled;
      desc.RenderTarget[i].RenderTargetWriteMask = spToD3D11(attachment.m_eColorWriteMask);
      desc.RenderTarget[i].SrcBlend = spToD3D11(attachment.m_eSourceColorBlendFactor);
      desc.RenderTarget[i].DestBlend = spToD3D11(attachment.m_eDestinationColorBlendFactor);
      desc.RenderTarget[i].BlendOp = spToD3D11(attachment.m_eColorBlendFunction);
      desc.RenderTarget[i].SrcBlendAlpha = spToD3D11(attachment.m_eSourceAlphaBlendFactor);
      desc.RenderTarget[i].DestBlendAlpha = spToD3D11(attachment.m_eDestinationAlphaBlendFactor);
      desc.RenderTarget[i].BlendOpAlpha = spToD3D11(attachment.m_eAlphaBlendFunction);

      i++;
    }

    for (; i < 8; ++i)
    {
      desc.RenderTarget[i].BlendEnable = false;
      desc.RenderTarget[i].RenderTargetWriteMask = 0;
      desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ZERO;
      desc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
      desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
      desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
      desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
      desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }

    desc.AlphaToCoverageEnable = blendState.m_bAlphaToCoverage;
    desc.IndependentBlendEnable = true;

    ID3D11BlendState* pBlendState = nullptr;
    const HRESULT res = m_pD3D11Device->CreateBlendState(&desc, &pBlendState);
    EZ_HRESULT_TO_ASSERT(res);

    return pBlendState;
  }

  ID3D11DepthStencilState* spDeviceResourceManagerD3D11::CreateDepthStencilState(const spDepthState& depthState, const spStencilState& stencilState) const
  {
    D3D11_DEPTH_STENCIL_DESC desc;

    desc.DepthFunc = spToD3D11(depthState.m_eDepthStencilComparison);
    desc.DepthEnable = depthState.m_bDepthTestEnabled;
    desc.DepthWriteMask = depthState.m_bDepthMaskEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.StencilEnable = stencilState.m_bEnabled;
    desc.FrontFace.StencilDepthFailOp = spToD3D11(stencilState.m_Front.m_eDepthFail);
    desc.FrontFace.StencilFailOp = spToD3D11(stencilState.m_Front.m_eFail);
    desc.FrontFace.StencilPassOp = spToD3D11(stencilState.m_Front.m_ePass);
    desc.FrontFace.StencilFunc = spToD3D11(stencilState.m_Front.m_eComparison);
    desc.BackFace.StencilDepthFailOp = spToD3D11(stencilState.m_Back.m_eDepthFail);
    desc.BackFace.StencilFailOp = spToD3D11(stencilState.m_Back.m_eFail);
    desc.BackFace.StencilPassOp = spToD3D11(stencilState.m_Back.m_ePass);
    desc.BackFace.StencilFunc = spToD3D11(stencilState.m_Back.m_eComparison);
    desc.StencilReadMask = stencilState.m_uiReadMask;
    desc.StencilWriteMask = stencilState.m_uiWriteMask;

    ID3D11DepthStencilState* pDepthStencilState = nullptr;
    const HRESULT res = m_pD3D11Device->CreateDepthStencilState(&desc, &pDepthStencilState);
    EZ_HRESULT_TO_ASSERT(res);

    return pDepthStencilState;
  }

  ID3D11RasterizerState* spDeviceResourceManagerD3D11::CreateRasterizerState(const spRasterizerState& rasterizerState, bool bMultisample) const
  {
    if (rasterizerState.m_bConservativeRasterization)
    {
      if (!m_pDevice->GetCapabilities().m_bConservativeRasterization)
      {
        ezLog::Error("Conservative rasterization is not supported on this device.");
        return nullptr;
      }

      D3D11_RASTERIZER_DESC2 desc;

      desc.CullMode = spToD3D11(rasterizerState.m_eFaceCulling);
      desc.FillMode = spToD3D11(rasterizerState.m_ePolygonFillMode);
      desc.DepthClipEnable = rasterizerState.m_bDepthClipEnabled;
      desc.ScissorEnable = rasterizerState.m_bScissorTestEnabled;
      desc.FrontCounterClockwise = rasterizerState.m_eFrontFace == spFrontFace::CounterClockwise;
      desc.MultisampleEnable = bMultisample;
      desc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON;
      desc.ForcedSampleCount = 0;
      desc.DepthBias = static_cast<INT>(rasterizerState.m_fDepthBias);
      desc.DepthBiasClamp = rasterizerState.m_fDepthBiasClamp;
      desc.SlopeScaledDepthBias = rasterizerState.m_fSlopeScaledDepthBias;

      ID3D11RasterizerState2* pRasterizerState = nullptr;
      const HRESULT res = m_pD3D11Device->CreateRasterizerState2(&desc, &pRasterizerState);
      EZ_HRESULT_TO_ASSERT(res);

      return pRasterizerState;
    }

    D3D11_RASTERIZER_DESC desc;

    desc.CullMode = spToD3D11(rasterizerState.m_eFaceCulling);
    desc.FillMode = spToD3D11(rasterizerState.m_ePolygonFillMode);
    desc.DepthClipEnable = rasterizerState.m_bDepthClipEnabled;
    desc.ScissorEnable = rasterizerState.m_bScissorTestEnabled;
    desc.FrontCounterClockwise = rasterizerState.m_eFrontFace == spFrontFace::CounterClockwise;
    desc.MultisampleEnable = bMultisample;
    desc.DepthBias = static_cast<INT>(rasterizerState.m_fDepthBias);
    desc.DepthBiasClamp = rasterizerState.m_fDepthBiasClamp;
    desc.SlopeScaledDepthBias = rasterizerState.m_fSlopeScaledDepthBias;

    ID3D11RasterizerState* pRasterizerState = nullptr;
    const HRESULT res = m_pD3D11Device->CreateRasterizerState(&desc, &pRasterizerState);
    EZ_HRESULT_TO_ASSERT(res);

    return pRasterizerState;
  }

  ID3D11InputLayout* spDeviceResourceManagerD3D11::CreateInputLayout(ezArrayPtr<spInputLayoutDescription> inputLayouts, ezByteArrayPtr vertexShaderByteCode) const
  {
    ezUInt32 uiTotalCount = 0;
    for (ezUInt32 i = 0, l = inputLayouts.GetCount(); i < l; i++)
      uiTotalCount += inputLayouts[i].m_Elements.GetCount();

    ezUInt32 uiElement = 0; // Total element index across slots
    ezHybridArray<D3D11_INPUT_ELEMENT_DESC, spInputElementLocationSemantic::Last> elements;
    SemanticIndices semanticIndices{};

    elements.SetCount(uiTotalCount);

    for (ezUInt32 slot = 0, l = inputLayouts.GetCount(); slot < l; slot++)
    {
      const auto& inputLayout = inputLayouts[slot];

      const ezUInt32 uiStepRate = inputLayout.m_uiInstanceStepRate;
      ezUInt32 uiCurrentOffset = 0;

      for (ezUInt32 i = 0, c = inputLayout.m_Elements.GetCount(); i < c; i++, uiElement++)
      {
        const auto& element = inputLayout.m_Elements[uiElement];

        elements[uiElement].AlignedByteOffset = element.m_uiOffset != 0 ? element.m_uiOffset : uiCurrentOffset;
        elements[uiElement].Format = spToD3D11(element.m_eFormat);
        elements[uiElement].InputSlot = slot;
        elements[uiElement].InputSlotClass = uiStepRate == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
        elements[uiElement].InstanceDataStepRate = uiStepRate;
        elements[uiElement].SemanticName = GetSemanticName(element.m_eSemantic);
        elements[uiElement].SemanticIndex = SemanticIndices::GetAndIncrement(semanticIndices, element.m_eSemantic);

        uiCurrentOffset += spPixelFormatHelper::GetSizeInBytes(element.m_eFormat);
      }
    }

    ID3D11InputLayout* pInputLayout = nullptr;
    const HRESULT res = m_pD3D11Device->CreateInputLayout(&elements[0], uiTotalCount, vertexShaderByteCode.GetPtr(), vertexShaderByteCode.GetCount(), &pInputLayout);
    EZ_HRESULT_TO_ASSERT(res);

    return pInputLayout;
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_ResourceManager);
