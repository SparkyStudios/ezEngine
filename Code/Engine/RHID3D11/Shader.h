#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Shader.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spShaderD3D11 final : public spShader, public spDeferredDeviceResource
{
  friend class spDeviceResourceFactoryD3D11;

public:
  // spDeviceResource

  void ReleaseResource() override;
  bool IsReleased() const override;
  void SetDebugName(const ezString& debugName) override;

  // spDeferredDeviceResource

  void CreateResource() override;

  // spShaderD3D11

  spShaderD3D11(spDeviceD3D11* pDevice, const spShaderDescription& description);
  ~spShaderD3D11() override;

private:
  ID3D11Device* m_pD3D11Device{nullptr};

  ID3D11DeviceChild* m_pD3D11Shader{nullptr};
  ezByteArrayPtr m_pByteCode;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spShaderD3D11);
