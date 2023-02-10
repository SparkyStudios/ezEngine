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

  // spSamplerStateD3D11

public:
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11SamplerState* GetD3D11SamplerState() const { return m_pSamplerState; }

private:
  spSamplerStateD3D11(spDeviceD3D11* pDevice, const spSamplerDescription& description);

  spSamplerDescription m_Description;
  ID3D11SamplerState* m_pSamplerState{nullptr};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spSamplerStateD3D11);

class SP_RHID3D11_DLL spSamplerD3D11 : public spSampler, public spDeferredDeviceResource
{
  friend class spDeviceResourceFactoryD3D11;

  // spDeviceResource

public:
  void SetDebugName(const ezString& name) override;
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spDefferredDeviceResource

public:
  void CreateResource() override;

  // spSampler

public:
  EZ_NODISCARD spResourceHandle GetSamplerWithMipMap() const override;
  EZ_NODISCARD spResourceHandle GetSamplerWithoutMipMap() const override;

  // spSamplerD3D11

public:
  ~spSamplerD3D11() override;

  EZ_NODISCARD EZ_ALWAYS_INLINE spSamplerStateD3D11* GetSamplerState() const { return m_pSamplerState; }

private:
  spSamplerD3D11(spDeviceD3D11* pDevice, const spSamplerDescription& description);

  spSamplerStateD3D11* m_pSamplerState;

  spSamplerDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spSamplerD3D11);
