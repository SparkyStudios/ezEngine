#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Device.h>
#include <RHIMTL/ResourceLayout.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spResourceLayoutMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spResourceLayoutMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_Bindings.Clear();
  }

  bool spResourceLayoutMTL::IsReleased() const
  {
    // Technically, a resource layout cannot be empty, so an empty resource layout is considered released.
    return m_Bindings.IsEmpty();
  }

  const spResourceLayoutMTL::BindingInfo& spResourceLayoutMTL::GetBinding(ezUInt32 uiSlot) const
  {
    EZ_ASSERT_DEV(uiSlot < m_Bindings.GetCount(), "Invalid binding index for resource layout.");
    return m_Bindings[uiSlot];
  }

  bool spResourceLayoutMTL::IsDynamicBuffer(ezUInt32 uiSlot) const
  {
    EZ_ASSERT_DEV(uiSlot < m_Bindings.GetCount(), "Invalid binding index for resource layout.");
    return m_Bindings[uiSlot].m_bDynamicBuffer;
  }

  spResourceLayoutMTL::spResourceLayoutMTL(spDeviceMTL* pDevice, const spResourceLayoutDescription& description)
    : spResourceLayout(description)
  {
    m_pDevice = pDevice;

    const auto& elements = description.m_Elements;
    m_Bindings.EnsureCount(elements.GetCount());

    ezUInt32 uiBufferIndex = 0;
    ezUInt32 uiTextureIndex = 0;
    ezUInt32 uiSamplerIndex = 0;

    for (ezUInt32 i = 0, l = m_Bindings.GetCount(); i < l; ++i)
    {
      ezUInt32 uiResourceSlot = 0;

      switch (elements[i].m_eType)
      {
        case spShaderResourceType::ConstantBuffer:
        case spShaderResourceType::ReadOnlyStructuredBuffer:
        case spShaderResourceType::ReadWriteStructuredBuffer:
          uiResourceSlot = uiBufferIndex++;
          break;

        case spShaderResourceType::ReadOnlyTexture:
        case spShaderResourceType::ReadWriteTexture:
          uiResourceSlot = uiTextureIndex++;
          break;

        case spShaderResourceType::Sampler:
          uiResourceSlot = uiSamplerIndex++;
          break;

        default:
          EZ_ASSERT_NOT_IMPLEMENTED;
          break;
      }

      m_Bindings[i].m_uiSlot = i;
      m_Bindings[i].m_uiResourceSlot = uiResourceSlot;
      m_Bindings[i].m_eShaderStage = description.m_eShaderStage;
      m_Bindings[i].m_eResourceType = elements[i].m_eType;
      m_Bindings[i].m_bDynamicBuffer = elements[i].m_eOptions.IsSet(spResourceLayoutElementOptions::DynamicBinding);
    }

    m_uiSamplerCount = uiSamplerIndex;
    m_uiTextureCount = uiTextureIndex;
    m_uiBufferCount = uiBufferIndex;
  }

  spResourceLayoutMTL::~spResourceLayoutMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_ResourceLayout);
