#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Device.h>
#include <RHIMTL/ResourceLayout.h>
#include <RHIMTL/ResourceSet.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spResourceSetMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spResourceSetMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_pLayout.Clear();

    for (auto& pResource : m_Resources)
      pResource.value.Clear();

    m_pLayout = nullptr;
    m_Resources.Clear();
  }

  bool spResourceSetMTL::IsReleased() const
  {
    return m_pLayout == nullptr && m_Resources.IsEmpty();
  }

  spResourceSetMTL::spResourceSetMTL(spDeviceMTL* pDevice, const spResourceSetDescription& description)
    : spResourceSet(description)
  {
    m_pDevice = pDevice;

    m_pLayout = pDevice->GetResourceManager()->GetResource<spResourceLayoutMTL>(description.m_hResourceLayout);
    EZ_ASSERT_DEV(m_pLayout != nullptr, "Unable to find a resource layout for the resource set.");

    m_Resources.Reserve(description.m_BoundResources.GetCount());
    for (auto& hResource : description.m_BoundResources)
    {
      auto pResource = pDevice->GetResourceManager()->GetResource<spShaderResource>(hResource.value);
      EZ_ASSERT_DEV(pResource != nullptr, "Unable to find a resource for the resource set.");

      m_Resources[hResource.key] = pResource;
    }
  }

  spResourceSetMTL::~spResourceSetMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  ezSharedPtr<spBufferMTL> spResourceSetMTL::GetArgumentBuffer(const MTL::ArgumentEncoder* pArgumentEncoder)
  {
    if (m_ArgumentBuffer == nullptr)
    {
      m_ArgumentBuffer = m_pDevice->GetResourceFactory()->CreateBuffer(spBufferDescription(pArgumentEncoder->encodedLength(), spBufferUsage::Staging)).Downcast<spBufferMTL>();
      m_ArgumentBuffer->EnsureResourceCreated();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      ezStringBuilder sDebugName;
      sDebugName.SetFormat("{0}__ArgumentBuffer", m_sDebugName);
      m_ArgumentBuffer->SetDebugName(sDebugName);
#endif
    }

    return m_ArgumentBuffer;
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_ResourceSet);
