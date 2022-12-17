
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/Resource.h>

class EZ_RENDERERFOUNDATION_DLL ezGALUnorderedAccessView : public ezGALObject<ezGALUnorderedAccessViewCreationDescription>
{
public:
  EZ_ALWAYS_INLINE ezGALResourceBase* GetResource() const { return m_pResource; }

  EZ_ALWAYS_INLINE bool ShouldUnsetResourceView() const { return m_bUnsetResourceView; }

protected:
  friend class ezGALDevice;

  ezGALUnorderedAccessView(ezGALResourceBase* pResource, const ezGALUnorderedAccessViewCreationDescription& description);

  ~ezGALUnorderedAccessView() override;

  virtual ezResult InitPlatform(ezGALDevice* pDevice) = 0;

  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) = 0;

  ezGALResourceBase* m_pResource;
  bool m_bUnsetResourceView;
};
