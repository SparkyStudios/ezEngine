#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Texture.h>

class spTextureD3D11 final : public spTexture, public spDeferredDeviceResource
{
public:
  // spTexture

  void SetDebugName(const ezString& debugName) override;
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spDeferredDeviceResource

  void CreateResource() override;

  // spTextureD3D11

  spTextureD3D11(spDeviceD3D11* pDevice, const spTextureDescription& description);
  ~spTextureD3D11() override;

private:
  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11Resource* m_pTexture{nullptr};

  DXGI_FORMAT m_eFormat;
  DXGI_FORMAT m_eTypelessFormat;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spTextureD3D11);
