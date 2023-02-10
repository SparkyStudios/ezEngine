#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/ResourceSet.h>

class spResourceLayoutD3D11;
class spDeviceD3D11;

class SP_RHID3D11_DLL spResourceSetD3D11 final : public spResourceSet
{
  friend class spDeviceResourceFactoryD3D11;

public:
  // spDeviceResource

  void ReleaseResource() override;
  bool IsReleased() const override;

  // spResourceSetD3D11

  spResourceSetD3D11(spDeviceD3D11* pDevice, const spResourceSetDescription& description);
  ~spResourceSetD3D11() override;

  EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<spShaderResource*> GetResources() const { return m_Resources; }

private:
  spResourceLayoutD3D11* m_pLayout{nullptr};
  ezDynamicArray<spShaderResource*> m_Resources;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spResourceSetD3D11);
