#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/ResourceLayout.h>

void spResourceLayoutD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  m_Bindings.Clear();
}

bool spResourceLayoutD3D11::IsReleased() const
{
  // Technically, a resource layout cannot be empty, so an empty resource layout is considered released.
  return m_Bindings.IsEmpty();
}

spResourceLayoutD3D11::BindingInfo spResourceLayoutD3D11::GetBinding(ezUInt32 uiSlot) const
{
  EZ_ASSERT_DEV(uiSlot < m_Bindings.GetCount(), "Invalid binding index for resource layout.");
  return m_Bindings[uiSlot];
}

bool spResourceLayoutD3D11::IsDynamicBuffer(ezUInt32 uiSlot) const
{
  EZ_ASSERT_DEV(uiSlot < m_Bindings.GetCount(), "Invalid binding index for resource layout.");
  return m_Bindings[uiSlot].m_bDynamicBuffer;
}

spResourceLayoutD3D11::spResourceLayoutD3D11(spDeviceD3D11* pDevice, const spResourceLayoutDescription& description)
  : spResourceLayout(description)
{
  m_pDevice = pDevice;

  const auto& elements = description.m_Elements;
  m_Bindings.EnsureCount(elements.GetCount());

  ezUInt32 uiCBIndex = 0;
  ezUInt32 uiTexIndex = 0;
  ezUInt32 uiSamplerIndex = 0;
  ezUInt32 uiUAVIndex = 0;

  for (ezUInt32 i = 0, l = m_Bindings.GetCount(); i < l; ++i)
  {
    ezUInt32 uiSlot = 0;

    switch (elements[i].m_eType)
    {
      case spShaderResourceType::ConstantBuffer:
        uiSlot = uiCBIndex++;
        break;

      case spShaderResourceType::ReadOnlyStructuredBuffer:
      case spShaderResourceType::ReadOnlyTexture:
        uiSlot = uiTexIndex++;
        break;

      case spShaderResourceType::ReadWriteStructuredBuffer:
      case spShaderResourceType::ReadWriteTexture:
        uiSlot = uiUAVIndex++;
        break;

      case spShaderResourceType::Sampler:
        uiSlot = uiSamplerIndex++;
        break;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
    }

    m_Bindings[i].m_uiSlot = uiSlot;
    m_Bindings[i].m_eShaderStage = elements[i].m_eShaderStage;
    m_Bindings[i].m_eResourceType = elements[i].m_eType;
    m_Bindings[i].m_bDynamicBuffer = elements[i].m_eOptions.IsSet(spResourceLayoutElementOptions::DynamicBinding);
  }

  m_uiSamplerCount = uiSamplerIndex;
  m_uiTextureCount = uiTexIndex;
  m_uiStorageBufferCount = uiUAVIndex;
  m_uiConstantBufferCount = uiCBIndex;
}

spResourceLayoutD3D11::~spResourceLayoutD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_ResourceLayout);
