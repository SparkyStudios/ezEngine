#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Core.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Sampler.h>

namespace RHI
{
#pragma region spSamplerStateMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSamplerStateMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spSamplerDescription spSamplerStateMTL::GetSamplerDescription() const
  {
    return m_Description;
  }

  void spSamplerStateMTL::SetDebugName(ezStringView sDebugName)
  {
    spSamplerState::SetDebugName(sDebugName);

    if (IsReleased() || sDebugName.IsEmpty())
      return;

    const ezStringBuilder sName(sDebugName);

    NS::String* pString = NS::String::string(sName.GetData(), NS::UTF8StringEncoding);
    // m_pSamplerState->setLabel(pString);
    SP_RHI_MTL_RELEASE(pString);
  }

  void spSamplerStateMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_MTL_RELEASE(m_pSamplerState);
  }

  bool spSamplerStateMTL::IsReleased() const
  {
    return m_pSamplerState == nullptr;
  }

  spSamplerStateMTL::spSamplerStateMTL(spDeviceMTL* pDevice, const spSamplerDescription& description)
    : m_Description(description)
  {
    m_pDevice = pDevice;
    m_pSamplerState = nullptr;

    spScopedMTLResource pSamplerDesc(MTL::SamplerDescriptor::alloc()->init());
    pSamplerDesc->setSAddressMode(spToMTL(description.m_eAddressModeS));
    pSamplerDesc->setTAddressMode(spToMTL(description.m_eAddressModeT));
    pSamplerDesc->setRAddressMode(spToMTL(description.m_eAddressModeR));
    pSamplerDesc->setMinFilter(spToMTL(description.m_eMinFilter));
    pSamplerDesc->setMagFilter(spToMTL(description.m_eMagFilter));
    pSamplerDesc->setMipFilter(spToMTLMipFilter(description.m_eMipFilter));
    pSamplerDesc->setLodMinClamp(description.m_uiMinLod);
    pSamplerDesc->setLodMaxClamp(description.m_uiMaxLod);
    pSamplerDesc->setMaxAnisotropy(ezMath::Max<ezUInt8>(1, description.m_uiMaxAnisotropy));
    pSamplerDesc->setCompareFunction(spToMTL(description.m_eSamplerComparison));

    if (pDevice->GetSupportedFeatures().IsMacOS())
      pSamplerDesc->setBorderColor(spToMTL(description.m_BorderColor));

    m_pSamplerState = pDevice->GetMTLDevice()->newSamplerState(*pSamplerDesc);
  }

  spSamplerStateMTL::~spSamplerStateMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion

#pragma region spSamplerMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSamplerMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  ezSharedPtr<spSamplerState> spSamplerMTL::GetSamplerWithMipMap() const
  {
    return m_pSamplerState;
  }

  ezSharedPtr<spSamplerState> spSamplerMTL::GetSamplerWithoutMipMap() const
  {
    return m_pSamplerState;
  }

  void spSamplerMTL::CreateResource()
  {
    if (!IsReleased())
      return;

    m_pSamplerState = EZ_NEW(m_pDevice->GetAllocator(), spSamplerStateMTL, ezStaticCast<spDeviceMTL*>(m_pDevice), m_Description);
    m_pDevice->GetResourceManager()->RegisterResource(m_pSamplerState);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    SetDebugName(m_sDebugName);
#endif

    m_bIsResourceCreated = true;
  }

  void spSamplerMTL::SetDebugName(ezStringView sDebugName)
  {
    spDeviceResource::SetDebugName(sDebugName);

    if (!IsReleased())
      m_pSamplerState->SetDebugName(sDebugName);
  }

  void spSamplerMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_pDevice->GetResourceManager()->ReleaseResource(m_pSamplerState);

    m_bIsResourceCreated = false;
  }

  bool spSamplerMTL::IsReleased() const
  {
    return m_pSamplerState == nullptr || m_pSamplerState->IsReleased();
  }

  spSamplerMTL::~spSamplerMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  spSamplerMTL::spSamplerMTL(spDeviceMTL* pDevice, const spSamplerDescription& description)
    : m_pSamplerState(nullptr)
  {
    m_pDevice = pDevice;
    m_Description = description;
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Sampler);
