#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Texture.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spTextureD3D11 final : public spTexture, public spDeferredDeviceResource
{
  friend class spTextureViewD3D11;

  EZ_ADD_DYNAMIC_REFLECTION(spTextureD3D11, spTexture);

public:
  // spDeviceResource

  void SetDebugName(ezStringView sDebugName) override;
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spDeferredDeviceResource

  void CreateResource() override;

  // spTextureD3D11

  static ezSharedPtr<spTextureD3D11> FromExisting(const ezSharedPtr<spTextureD3D11>& pTexture);
  static ezSharedPtr<spTextureD3D11> FromNative(spDeviceD3D11* pDevice, ID3D11Texture2D* pTexture, const ezEnum<spTextureDimension>& eDimension, const ezEnum<spPixelFormat>& eFormat);

  spTextureD3D11(spDeviceD3D11* pDevice, const spTextureDescription& description);
  ~spTextureD3D11() override;

  EZ_NODISCARD ID3D11Resource* GetD3D11Texture() const;
  EZ_NODISCARD EZ_ALWAYS_INLINE DXGI_FORMAT GetDXGIFormat() const { return m_eFormat; }

private:
  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11Resource* m_pTexture{nullptr};

  bool m_bFromNative{false};

  DXGI_FORMAT m_eFormat{DXGI_FORMAT_UNKNOWN};
  DXGI_FORMAT m_eTypelessFormat{DXGI_FORMAT_UNKNOWN};

  ezSharedPtr<spTextureD3D11> m_pParentTexture{nullptr};
};

class SP_RHID3D11_DLL spTextureViewD3D11 : public spTextureView, public spDeferredDeviceResource
{
  EZ_ADD_DYNAMIC_REFLECTION(spTextureViewD3D11, spTextureView);

public:
  // spDeviceResource

  void SetDebugName(ezStringView sDebugName) override;
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spDeferredDeviceResource

  void CreateResource() override;

  // spTextureViewD3D11

  spTextureViewD3D11(spDeviceD3D11* pDevice, const spTextureViewDescription& description);
  ~spTextureViewD3D11() override;

  EZ_NODISCARD ID3D11ShaderResourceView* GetShaderResourceView() const;
  EZ_NODISCARD ID3D11UnorderedAccessView* GetUnorderedAccessView() const;

private:
  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11ShaderResourceView* m_pShaderResourceView{nullptr};
  ID3D11UnorderedAccessView* m_pUnorderedAccessView{nullptr};
};

class SP_RHID3D11_DLL spTextureSamplerManagerD3D11 final : public spTextureSamplerManager
{
  // spTextureSamplerManager

public:
  ezSharedPtr<spTextureView> GetFullTextureView(ezSharedPtr<spTexture> pTexture) override;

  // spTextureSamplerManagerD3D11

public:
  explicit spTextureSamplerManagerD3D11(spDeviceD3D11* pDevice);
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spTextureSamplerManagerD3D11);
