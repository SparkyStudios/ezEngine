#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Rendering.h>
#include <RHI/Resource.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spDeviceResourceManagerD3D11 final : public spDefaultDeviceResourceManager
{
  // spDeviceResourceManagerD3D11

public:
  explicit spDeviceResourceManagerD3D11(spDeviceD3D11* pDevice);
  ~spDeviceResourceManagerD3D11() override;

  void GetPipelineResources(const spBlendState& blendState, const spDepthState& depthState, const spStencilState& stencilState, const spRasterizerState& rasterizerState, bool bMultisample, ezArrayPtr<spInputLayoutDescription> inputLayouts, ezByteArrayPtr vertexShaderByteCode, ID3D11BlendState*& pBlendState, ID3D11DepthStencilState*& pDepthStencilState, ID3D11RasterizerState*& pRasterizerState, ID3D11InputLayout*& pInputLayouts);

private:
  struct RasterizerStateCacheKey
  {
    ezUInt32 m_uiDescriptionHash{0};
    bool m_bMultisample{false};

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const RasterizerStateCacheKey& rhs) const
    {
      return m_uiDescriptionHash == rhs.m_uiDescriptionHash && m_bMultisample == rhs.m_bMultisample;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const RasterizerStateCacheKey& rhs) const { return !(*this == rhs); }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator<(const RasterizerStateCacheKey& rhs) const
    {
      if (m_bMultisample == rhs.m_bMultisample)
        return m_bMultisample < rhs.m_bMultisample;

      return !m_bMultisample;
    }
  };

  struct InputLayoutCacheKey
  {
    ezArrayPtr<spInputLayoutDescription> m_InputLayouts;
    bool bOwned{false};


    EZ_NODISCARD EZ_ALWAYS_INLINE static InputLayoutCacheKey GetTemporaryKey(ezArrayPtr<spInputLayoutDescription> inputLayouts)
    {
      InputLayoutCacheKey key;
      key.m_InputLayouts = inputLayouts;

      return key;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE static InputLayoutCacheKey GetPermanentKey(ezArrayPtr<spInputLayoutDescription> inputLayouts)
    {
      InputLayoutCacheKey key;

      key.m_InputLayouts = EZ_DEFAULT_NEW_ARRAY(spInputLayoutDescription, inputLayouts.GetCount());
      key.m_InputLayouts.CopyFrom(inputLayouts);

      key.bOwned = true;

      return key;
    }

    ~InputLayoutCacheKey()
    {
      if (bOwned)
        EZ_DEFAULT_DELETE_ARRAY(m_InputLayouts);
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const InputLayoutCacheKey& rhs) const
    {
      return m_InputLayouts == rhs.m_InputLayouts;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const InputLayoutCacheKey& rhs) const { return !(*this == rhs); }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator<(const InputLayoutCacheKey& rhs) const
    {
      return m_InputLayouts.GetCount() < rhs.m_InputLayouts.GetCount();
    }
  };

  struct SemanticIndices
  {
    ezUInt32 m_uiPosition;
    ezUInt32 m_uiNormal;
    ezUInt32 m_uiTangent;
    ezUInt32 m_uiBiTangent;
    ezUInt32 m_uiTexCoord0;
    ezUInt32 m_uiTexCoord1;
    ezUInt32 m_uiColor0;
    ezUInt32 m_uiColor1;
    ezUInt32 m_uiBoneWeights0;
    ezUInt32 m_uiBoneIndices0;

    static ezUInt32 GetAndIncrement(SemanticIndices& indices, const ezEnum<spInputElementLocationSemantic>& eSemantic)
    {
      switch (eSemantic)
      {
        case spInputElementLocationSemantic::Position:
          return indices.m_uiPosition++;
        case spInputElementLocationSemantic::Normal:
          return indices.m_uiNormal++;
        case spInputElementLocationSemantic::Tangent:
          return indices.m_uiTangent++;
        case spInputElementLocationSemantic::BiTangent:
          return indices.m_uiBiTangent++;
        case spInputElementLocationSemantic::TexCoord0:
          return indices.m_uiTexCoord0++;
        case spInputElementLocationSemantic::TexCoord1:
          return indices.m_uiTexCoord1++;
        case spInputElementLocationSemantic::Color0:
          return indices.m_uiColor0++;
        case spInputElementLocationSemantic::Color1:
          return indices.m_uiColor1++;
        case spInputElementLocationSemantic::BoneWeights0:
          return indices.m_uiBoneWeights0++;
        case spInputElementLocationSemantic::BoneIndices0:
          return indices.m_uiBoneIndices0++;
        default:
          EZ_ASSERT_NOT_IMPLEMENTED;
          return 0;
      }
    }
  };

  ID3D11BlendState* GetBlendState(const spBlendState& blendState);
  ID3D11DepthStencilState* GetDepthStencilState(const spDepthState& depthState, const spStencilState& stencilState);
  ID3D11RasterizerState* GetRasterizerState(const spRasterizerState& rasterizerState, bool bMultisample);
  ID3D11InputLayout* GetInputLayout(ezArrayPtr<spInputLayoutDescription> inputLayouts, ezByteArrayPtr vertexShaderByteCode);

  ID3D11BlendState* CreateBlendState(const spBlendState& blendState);
  ID3D11DepthStencilState* CreateDepthStencilState(const spDepthState& depthState, const spStencilState& stencilState);
  ID3D11RasterizerState* CreateRasterizerState(const spRasterizerState& rasterizerState, bool bMultisample);
  ID3D11InputLayout* CreateInputLayout(ezArrayPtr<spInputLayoutDescription> inputLayouts, ezByteArrayPtr vertexShaderByteCode);

  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11Device3* m_pD3D11Device3{nullptr};

  ezArrayMap<ezUInt32, ID3D11BlendState*> m_BlendStates;
  ezArrayMap<ezUInt32, ID3D11DepthStencilState*> m_DepthStencilStates;
  ezArrayMap<RasterizerStateCacheKey, ID3D11RasterizerState*> m_RasterizerStates;
  ezArrayMap<InputLayoutCacheKey, ID3D11InputLayout*> m_InputLayouts;

  ezMutex m_Mutex;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spDeviceResourceManagerD3D11);
