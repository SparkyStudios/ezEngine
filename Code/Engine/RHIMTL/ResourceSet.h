#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/ResourceSet.h>

namespace RHI
{
  class spResourceLayoutMTL;
  class spDeviceMTL;

  class SP_RHIMTL_DLL spResourceSetMTL final : public spResourceSet
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spResourceSetMTL, spResourceSet);

  public:
    // spDeviceResource

    void ReleaseResource() override;
    bool IsReleased() const override;

    // spResourceSetMTL

    spResourceSetMTL(spDeviceMTL* pDevice, const spResourceSetDescription& description);
    ~spResourceSetMTL() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezArrayMap<ezHashedString, ezSharedPtr<spShaderResource>>& GetResources() const { return m_Resources; }

  private:
    ezSharedPtr<spResourceLayoutMTL> m_pLayout{nullptr};
    ezArrayMap<ezHashedString, ezSharedPtr<spShaderResource>> m_Resources;
  };
}
