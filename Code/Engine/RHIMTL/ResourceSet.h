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

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spResourceSetMTL

  public:
    spResourceSetMTL(spDeviceMTL* pDevice, const spResourceSetDescription& description);
    ~spResourceSetMTL() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezArrayMap<ezTempHashedString, ezSharedPtr<spShaderResource>>& GetResources() const { return m_Resources; }

    EZ_NODISCARD ezSharedPtr<spBufferMTL> GetArgumentBuffer(const MTL::ArgumentEncoder* pArgumentEncoder);

  private:
    ezSharedPtr<spResourceLayoutMTL> m_pLayout{nullptr};
    ezArrayMap<ezTempHashedString, ezSharedPtr<spShaderResource>> m_Resources;

    ezSharedPtr<spBufferMTL> m_ArgumentBuffer{nullptr};
  };
} // namespace RHI
