#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Shader.h>

class spDeviceD3D11;
class spShaderD3D11;

class SP_RHID3D11_DLL spShaderProgramD3D11 final : public spShaderProgram
{
  // spDeviceResource
public:
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spShaderProgram

public:
  void Attach(const spResourceHandle& hShader) override;
  void Detach(const spResourceHandle& hShader) override;
  void Detach(const ezEnum<spShaderStage>& eStage) override;
  void DetachAll() override;
  void Use() override;
  EZ_NODISCARD spResourceHandle Get(const ezEnum<spShaderStage>& eStage) const override;

  // spShaderProgramD3D11

public:
  spShaderProgramD3D11(spDeviceD3D11* pDevice);
  ~spShaderProgramD3D11();

  EZ_NODISCARD EZ_ALWAYS_INLINE spShaderD3D11* GetVertexShader() const { return m_pVertexShader; }
  EZ_NODISCARD EZ_ALWAYS_INLINE spShaderD3D11* GetGeometryShader() const { return m_pGeometryShader; }
  EZ_NODISCARD EZ_ALWAYS_INLINE spShaderD3D11* GetHullShader() const { return m_pHullShader; }
  EZ_NODISCARD EZ_ALWAYS_INLINE spShaderD3D11* GetDomainShader() const { return m_pDomainShader; }
  EZ_NODISCARD EZ_ALWAYS_INLINE spShaderD3D11* GetPixelShader() const { return m_pPixelShader; }
  EZ_NODISCARD EZ_ALWAYS_INLINE spShaderD3D11* GetComputeShader() const { return m_pComputeShader; }

private:
  ID3D11Device* m_pD3D11Device{nullptr};

  spShaderD3D11* m_pVertexShader{nullptr};
  spShaderD3D11* m_pGeometryShader{nullptr};
  spShaderD3D11* m_pHullShader{nullptr};
  spShaderD3D11* m_pDomainShader{nullptr};
  spShaderD3D11* m_pPixelShader{nullptr};
  spShaderD3D11* m_pComputeShader{nullptr};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spShaderProgramD3D11);

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

  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11DeviceChild* GetD3D11Shader() const { return m_pD3D11Shader; }

  EZ_NODISCARD EZ_ALWAYS_INLINE ezByteArrayPtr GetShaderByteCode() const { return m_pByteCode; }

private:
  ID3D11Device* m_pD3D11Device{nullptr};

  ID3D11DeviceChild* m_pD3D11Shader{nullptr};
  ezByteArrayPtr m_pByteCode;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spShaderD3D11);
