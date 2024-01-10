#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Sampler.h>

namespace RHI
{
  class spDeviceMTL;

  class SP_RHIMTL_DLL spSamplerStateMTL : public spSamplerState
  {
    friend class spSamplerMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spSamplerStateMTL, spSamplerState);

  public:
    spSamplerDescription GetSamplerDescription() const override;
    void SetDebugName(ezStringView sDebugName) override;
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spSamplerStateMTL

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::SamplerState* GetMTLSamplerState() const { return m_pSamplerState; }

    spSamplerStateMTL(spDeviceMTL* pDevice, const spSamplerDescription& description);
    ~spSamplerStateMTL() override;

    spSamplerDescription m_Description;
    MTL::SamplerState* m_pSamplerState{nullptr};
  };

  class SP_RHIMTL_DLL spSamplerMTL : public spSampler, public spDeferredDeviceResource
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spSamplerMTL, spSampler);

    // spDeviceResource

  public:
    void SetDebugName(ezStringView sDebugName) override;
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spDeferredDeviceResource

  public:
    void CreateResource() override;

    // spSampler

  public:
    EZ_NODISCARD ezSharedPtr<spSamplerState> GetSamplerWithMipMap() const override;
    EZ_NODISCARD ezSharedPtr<spSamplerState> GetSamplerWithoutMipMap() const override;

    // spSamplerMTL

  public:
    ~spSamplerMTL() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spSamplerStateMTL> GetSamplerState() const { return m_pSamplerState; }

  private:
    spSamplerMTL(spDeviceMTL* pDevice, const spSamplerDescription& description);

    ezSharedPtr<spSamplerStateMTL> m_pSamplerState;

    spSamplerDescription m_Description;
  };
} // namespace RHI
