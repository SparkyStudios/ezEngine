#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/ResourceLayout.h>
#include <RHID3D11/ResourceSet.h>

void spResourceSetD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  m_pLayout.Clear();

  for (auto& pResource : m_Resources)
    pResource.Clear();

  m_pLayout = nullptr;
  m_Resources.Clear();
}

bool spResourceSetD3D11::IsReleased() const
{
  return m_pLayout == nullptr && m_Resources.IsEmpty();
}

spResourceSetD3D11::spResourceSetD3D11(spDeviceD3D11* pDevice, const spResourceSetDescription& description)
  : spResourceSet(description)
{
  m_pDevice = pDevice;

  m_pLayout = pDevice->GetResourceManager()->GetResource<spResourceLayoutD3D11>(description.m_hResourceLayout);
  EZ_ASSERT_DEV(m_pLayout != nullptr, "Unable to find a resource layout for the resource set.");

  m_Resources.Reserve(description.m_BoundResources.GetCount());
  for (auto& hResource : description.m_BoundResources)
  {
    auto pResource = pDevice->GetResourceManager()->GetResource<spShaderResource>(hResource);
    EZ_ASSERT_DEV(pResource != nullptr, "Unable to find a resource for the resource set.");

    m_Resources.PushBack(pResource);
  }
}

spResourceSetD3D11::~spResourceSetD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}
