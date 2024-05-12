#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Buffer.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Fence.h>

namespace RHI
{
#pragma region spBufferMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spBufferMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spBufferMTL::SetDebugName(ezStringView sDebugName)
  {
    spBuffer::SetDebugName(sDebugName);

    if (IsReleased() || sDebugName.IsEmpty())
      return;

    const ezStringBuilder sName(sDebugName);

    NS::String* pString = NS::String::string(sName.GetData(), NS::ASCIIStringEncoding);
    m_pBuffer->setLabel(pString);
    SP_RHI_MTL_RELEASE(pString);
  }

  void spBufferMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    for (auto& range : m_BufferRanges)
      range.Clear();

    SP_RHI_MTL_RELEASE(m_pBuffer);

    m_BufferRanges.Clear();
    m_bIsResourceCreated = false;
  }

  void spBufferMTL::CreateResource()
  {
    PreCreateResource();

    const ezUInt32 roundFactor = (4 - (m_uiBufferAlignedSize % 4)) % 4;
    m_uiActualCapacity = m_uiBufferAlignedSize + roundFactor;

    const bool bIsSharedMemory = m_Description.m_eUsage.IsSet(spBufferUsage::Dynamic) || m_Description.m_eUsage.IsSet(spBufferUsage::IndirectBuffer) || m_Description.m_eUsage == spBufferUsage::Staging;

    m_pBuffer = m_pMTLDevice->newBuffer(
      m_uiActualCapacity * m_uiBufferCount,
      bIsSharedMemory ? MTL::ResourceStorageModeShared : MTL::ResourceStorageModePrivate);

    PostCreateResource();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    SetDebugName(m_sDebugName);
#endif

    m_bIsResourceCreated = true;
  }

  spBufferMTL::spBufferMTL(RHI::spDeviceMTL* pDevice, const RHI::spBufferDescription& description)
    : spBuffer(description)
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();
  }

  spBufferMTL::~spBufferMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion

#pragma region spBufferRangeMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spBufferRangeMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spBufferRangeMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_pBuffer = nullptr;
    m_pFence.Clear();

    m_bReleased = true;
  }

  void spBufferRangeMTL::CreateResource()
  {
    m_pBuffer->EnsureResourceCreated();
  }

  spBufferRangeMTL::spBufferRangeMTL(RHI::spDeviceMTL* pDevice, const RHI::spBufferRangeDescription& description)
    : spBufferRange(description)
  {
    m_pDevice = pDevice;

    m_pBuffer = pDevice->GetResourceManager()->GetResource<spBufferMTL>(description.m_hBuffer);
    EZ_ASSERT_DEV(m_pBuffer != nullptr, "Buffer range creation failed. Invalid parent buffer provided.");

    m_pFence = m_pDevice->GetResourceFactory()->CreateFence(description.m_Fence).Downcast<spFenceMTL>();

    m_bReleased = false;
  }

  spBufferRangeMTL::~spBufferRangeMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Buffer);
