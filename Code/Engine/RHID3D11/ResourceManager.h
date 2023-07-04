#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Input.h>
#include <RHI/Rendering.h>
#include <RHI/Resource.h>

namespace RHI
{
  class spDeviceD3D11;

  class SP_RHID3D11_DLL spDeviceResourceManagerD3D11 final : public spDefaultDeviceResourceManager
  {
    EZ_ADD_DYNAMIC_REFLECTION(spDeviceResourceManagerD3D11, spDefaultDeviceResourceManager);

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
      bool m_bOwned{false};

      EZ_NODISCARD EZ_ALWAYS_INLINE static InputLayoutCacheKey GetTemporaryKey(const ezArrayPtr<spInputLayoutDescription>& inputLayouts)
      {
        InputLayoutCacheKey key;
        key.m_InputLayouts = inputLayouts;

        return key;
      }

      EZ_NODISCARD EZ_ALWAYS_INLINE static InputLayoutCacheKey GetPermanentKey(const ezArrayPtr<spInputLayoutDescription>& inputLayouts)
      {
        InputLayoutCacheKey key;

        key.m_InputLayouts = EZ_DEFAULT_NEW_ARRAY(spInputLayoutDescription, inputLayouts.GetCount());
        ezMemoryUtils::Construct(key.m_InputLayouts.GetPtr(), key.m_InputLayouts.GetCount());

        for (ezUInt32 i = 0, l = key.m_InputLayouts.GetCount(); i < l; ++i)
        {
          key.m_InputLayouts[i].m_uiStride = inputLayouts[i].m_uiStride;
          key.m_InputLayouts[i].m_uiInstanceStepRate = inputLayouts[i].m_uiInstanceStepRate;

          key.m_InputLayouts[i].m_Elements.EnsureCount(inputLayouts[i].m_Elements.GetCount());
          for (ezUInt32 j = 0, m = key.m_InputLayouts[i].m_Elements.GetCount(); j < m; ++j)
          {
            key.m_InputLayouts[i].m_Elements[j].m_eFormat = inputLayouts[i].m_Elements[j].m_eFormat;
            key.m_InputLayouts[i].m_Elements[j].m_eSemantic = inputLayouts[i].m_Elements[j].m_eSemantic;
            key.m_InputLayouts[i].m_Elements[j].m_uiOffset = inputLayouts[i].m_Elements[j].m_uiOffset;
            key.m_InputLayouts[i].m_Elements[j].m_sName.Assign(inputLayouts[i].m_Elements[j].m_sName.GetData());
          }
        }

        key.m_bOwned = true;

        return key;
      }

      InputLayoutCacheKey() = default;

      InputLayoutCacheKey(const InputLayoutCacheKey& key)
      {
        m_InputLayouts = EZ_DEFAULT_NEW_ARRAY(spInputLayoutDescription, key.m_InputLayouts.GetCount());
        ezMemoryUtils::Construct(m_InputLayouts.GetPtr(), m_InputLayouts.GetCount());

        for (ezUInt32 i = 0, l = key.m_InputLayouts.GetCount(); i < l; ++i)
        {
          m_InputLayouts[i].m_uiStride = key.m_InputLayouts[i].m_uiStride;
          m_InputLayouts[i].m_uiInstanceStepRate = key.m_InputLayouts[i].m_uiInstanceStepRate;

          m_InputLayouts[i].m_Elements.EnsureCount(key.m_InputLayouts[i].m_Elements.GetCount());
          for (ezUInt32 j = 0, m = key.m_InputLayouts[i].m_Elements.GetCount(); j < m; ++j)
          {
            m_InputLayouts[i].m_Elements[j].m_eFormat = key.m_InputLayouts[i].m_Elements[j].m_eFormat;
            m_InputLayouts[i].m_Elements[j].m_eSemantic = key.m_InputLayouts[i].m_Elements[j].m_eSemantic;
            m_InputLayouts[i].m_Elements[j].m_uiOffset = key.m_InputLayouts[i].m_Elements[j].m_uiOffset;
            m_InputLayouts[i].m_Elements[j].m_sName.Assign(key.m_InputLayouts[i].m_Elements[j].m_sName.GetData());
          }
        }

        m_bOwned = true;
      }

      InputLayoutCacheKey(InputLayoutCacheKey&& key) noexcept
      {
        m_InputLayouts.Swap(key.m_InputLayouts);
        m_bOwned = key.m_bOwned;

        key.m_InputLayouts.Clear();
        key.m_bOwned = false;
      }

      ~InputLayoutCacheKey()
      {
        if (m_bOwned)
          EZ_DEFAULT_DELETE_ARRAY(m_InputLayouts);
      }

      EZ_ALWAYS_INLINE InputLayoutCacheKey& operator=(InputLayoutCacheKey&& rhs) noexcept
      {
        m_InputLayouts.Swap(rhs.m_InputLayouts);
        m_bOwned = rhs.m_bOwned;

        rhs.m_InputLayouts.Clear();
        rhs.m_bOwned = false;

        return *this;
      }

      EZ_ALWAYS_INLINE InputLayoutCacheKey& operator=(const InputLayoutCacheKey& rhs)
      {
        m_InputLayouts = EZ_DEFAULT_NEW_ARRAY(spInputLayoutDescription, rhs.m_InputLayouts.GetCount());
        ezMemoryUtils::Construct(m_InputLayouts.GetPtr(), m_InputLayouts.GetCount());

        for (ezUInt32 i = 0, l = rhs.m_InputLayouts.GetCount(); i < l; ++i)
        {
          m_InputLayouts[i].m_uiStride = rhs.m_InputLayouts[i].m_uiStride;
          m_InputLayouts[i].m_uiInstanceStepRate = rhs.m_InputLayouts[i].m_uiInstanceStepRate;

          m_InputLayouts[i].m_Elements.EnsureCount(rhs.m_InputLayouts[i].m_Elements.GetCount());
          for (ezUInt32 j = 0, m = rhs.m_InputLayouts[i].m_Elements.GetCount(); j < m; ++j)
          {
            m_InputLayouts[i].m_Elements[j].m_eFormat = rhs.m_InputLayouts[i].m_Elements[j].m_eFormat;
            m_InputLayouts[i].m_Elements[j].m_eSemantic = rhs.m_InputLayouts[i].m_Elements[j].m_eSemantic;
            m_InputLayouts[i].m_Elements[j].m_uiOffset = rhs.m_InputLayouts[i].m_Elements[j].m_uiOffset;
            m_InputLayouts[i].m_Elements[j].m_sName.Assign(rhs.m_InputLayouts[i].m_Elements[j].m_sName.GetData());
          }
        }

        m_bOwned = true;

        return *this;
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
      ezUInt32 m_uiTexCoord;
      ezUInt32 m_uiColor;
      ezUInt32 m_uiBoneWeights;
      ezUInt32 m_uiBoneIndices;

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
          case spInputElementLocationSemantic::TexCoord:
            return indices.m_uiTexCoord++;
          case spInputElementLocationSemantic::Color:
            return indices.m_uiColor++;
          case spInputElementLocationSemantic::BoneWeights:
            return indices.m_uiBoneWeights++;
          case spInputElementLocationSemantic::BoneIndices:
            return indices.m_uiBoneIndices++;
          default:
            EZ_ASSERT_NOT_IMPLEMENTED;
            return 0;
        }
      }
    };

    ID3D11BlendState* GetBlendState(const spBlendState& blendState);
    ID3D11DepthStencilState* GetDepthStencilState(const spDepthState& depthState, const spStencilState& stencilState);
    ID3D11RasterizerState* GetRasterizerState(const spRasterizerState& rasterizerState, bool bMultisample);
    ID3D11InputLayout* GetInputLayout(const ezArrayPtr<spInputLayoutDescription>& inputLayouts, const ezByteArrayPtr& vertexShaderByteCode);

    ID3D11BlendState* CreateBlendState(const spBlendState& blendState) const;
    ID3D11DepthStencilState* CreateDepthStencilState(const spDepthState& depthState, const spStencilState& stencilState) const;
    ID3D11RasterizerState* CreateRasterizerState(const spRasterizerState& rasterizerState, bool bMultisample) const;
    ID3D11InputLayout* CreateInputLayout(ezArrayPtr<spInputLayoutDescription> inputLayouts, ezByteArrayPtr vertexShaderByteCode) const;

    ID3D11Device5* m_pD3D11Device{nullptr};

    ezArrayMap<ezUInt32, ID3D11BlendState*> m_BlendStates;
    ezArrayMap<ezUInt32, ID3D11DepthStencilState*> m_DepthStencilStates;
    ezArrayMap<RasterizerStateCacheKey, ID3D11RasterizerState*> m_RasterizerStates;
    ezArrayMap<InputLayoutCacheKey, ID3D11InputLayout*> m_InputLayouts;

    ezMutex m_Mutex;
  };
} // namespace RHI
