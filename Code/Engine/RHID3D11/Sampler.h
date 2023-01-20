#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Sampler.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spSamplerStateD3D11 : public spSamplerState
{
  friend class spSamplerD3D11;

public:
  ~spSamplerStateD3D11() override;

  spSamplerDescription GetSamplerDescription() const override;
  void SetDebugName(const ezString& sDebugName) override;
  void ReleaseResource() override;
  bool IsReleased() const override;

private:
  spSamplerStateD3D11(spDeviceD3D11* pDevice, const spSamplerDescription& description);

  spSamplerDescription m_Description;
  ID3D11SamplerState* m_pSamplerState{nullptr};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spSamplerStateD3D11);

class SP_RHID3D11_DLL spSamplerD3D11 : public spSampler, public spDeferredDeviceResource
{
  friend class spDeviceResourceFactoryD3D11;

public:
  EZ_NODISCARD spResourceHandle GetSamplerWithMipMap() const override;
  EZ_NODISCARD spResourceHandle GetSamplerWithoutMipMap() const override;

  void CreateResource() override;

  void SetDebugName(const ezString& name) override;

  void ReleaseResource() override;

  bool IsReleased() const override;

private:
  spSamplerD3D11(spDeviceD3D11* pDevice, const spSamplerDescription& description);

  spSamplerStateD3D11* m_pSamplerState;

  spSamplerDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spSamplerD3D11);
