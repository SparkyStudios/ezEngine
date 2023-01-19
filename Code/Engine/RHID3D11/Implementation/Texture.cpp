#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Texture.h>

#pragma region spTexture

spTextureD3D11::spTextureD3D11(spDeviceD3D11* pDevice, const spTextureDescription& description)
  : spTexture(description)
  , m_pTexture(nullptr)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();

  m_eFormat = spToD3D11(description.m_eFormat, description.m_eUsage.IsSet(spTextureUsage::DepthStencil));
  m_eTypelessFormat = spGetTypelessFormat(m_eFormat);
}

spTextureD3D11::~spTextureD3D11()
{
  SP_RHI_DX11_RELEASE(m_pTexture);
}

void spTextureD3D11::SetDebugName(const ezString& debugName)
{
  spDeviceResource::SetDebugName(debugName);

  m_pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.GetElementCount(), debugName.GetData());
}

void spTextureD3D11::ReleaseResource()
{
  SP_RHI_DX11_RELEASE(m_pTexture);
}

bool spTextureD3D11::IsReleased() const
{
  return m_pTexture == nullptr;
}

void spTextureD3D11::CreateResource()
{
  UINT uiCPUFlags = 0;
  D3D11_USAGE eResourceUsage = D3D11_USAGE_DEFAULT;
  UINT uiBindFlags = 0;
  UINT uiOptionFlags = 0;

  if (m_Description.m_eUsage.IsSet(spTextureUsage::RenderTarget))
    uiBindFlags |= D3D11_BIND_RENDER_TARGET;

  if (m_Description.m_eUsage.IsSet(spTextureUsage::DepthStencil))
    uiBindFlags |= D3D11_BIND_DEPTH_STENCIL;

  if (m_Description.m_eUsage.IsSet(spTextureUsage::Sampled))
    uiBindFlags |= D3D11_BIND_SHADER_RESOURCE;

  if (m_Description.m_eUsage.IsSet(spTextureUsage::Storage))
    uiBindFlags |= D3D11_BIND_UNORDERED_ACCESS;

  if (m_Description.m_eUsage.IsSet(spTextureUsage::Staging))
  {
    uiCPUFlags |= D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    eResourceUsage = D3D11_USAGE_STAGING;
  }

  if (m_Description.m_eUsage.IsSet(spTextureUsage::GenerateMipmaps))
  {
    uiBindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    uiOptionFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
  }

  auto iArraySize = static_cast<ezInt32>(m_Description.m_uiArrayLayers);
  if (m_Description.m_eUsage.IsSet(spTextureUsage::Cubemap))
  {
    uiOptionFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    iArraySize *= 6;
  }

  auto iRoundedWidth = static_cast<ezInt32>(m_Description.m_uiWidth);
  auto iRoundedHeight = static_cast<ezInt32>(m_Description.m_uiHeight);
  if (spIsCompressedFormat(m_Description.m_eFormat))
  {
    iRoundedWidth = ((iRoundedWidth + 3) / 4) * 4;
    iRoundedHeight = ((iRoundedHeight + 3) / 4) * 4;
  }

  if (m_Description.m_eDimension == spTextureDimension::Texture1D)
  {
    D3D11_TEXTURE1D_DESC desc;
    desc.Width = iRoundedWidth;
    desc.MipLevels = m_Description.m_uiMipCount;
    desc.ArraySize = iArraySize;
    desc.Format = m_eTypelessFormat;
    desc.BindFlags = uiBindFlags;
    desc.CPUAccessFlags = uiCPUFlags;
    desc.Usage = eResourceUsage;
    desc.MiscFlags = uiOptionFlags;

    const HRESULT res = m_pD3D11Device->CreateTexture1D(&desc, nullptr, reinterpret_cast<ID3D11Texture1D**>(&m_pTexture));
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 texture: {}", (ezUInt32)HRESULT_CODE(res));
  }
  else if (m_Description.m_eDimension == spTextureDimension::Texture2D)
  {
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = iRoundedWidth;
    desc.Height = iRoundedHeight;
    desc.MipLevels = m_Description.m_uiMipCount;
    desc.ArraySize = iArraySize;
    desc.Format = m_eTypelessFormat;
    desc.BindFlags = uiBindFlags;
    desc.CPUAccessFlags = uiCPUFlags;
    desc.Usage = eResourceUsage;
    desc.SampleDesc = {m_Description.m_eSampleCount.GetValue(), 0};
    desc.MiscFlags = uiOptionFlags;

    const HRESULT res = m_pD3D11Device->CreateTexture2D(&desc, nullptr, reinterpret_cast<ID3D11Texture2D**>(&m_pTexture));
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 texture: {}", (ezUInt32)HRESULT_CODE(res));
  }
  else if (m_Description.m_eDimension == spTextureDimension::Texture3D)
  {
    D3D11_TEXTURE3D_DESC desc;
    desc.Width = iRoundedWidth;
    desc.Height = iRoundedHeight;
    desc.Depth = m_Description.m_uiDepth;
    desc.MipLevels = m_Description.m_uiMipCount;
    desc.Format = m_eTypelessFormat;
    desc.BindFlags = uiBindFlags;
    desc.CPUAccessFlags = uiCPUFlags;
    desc.Usage = eResourceUsage;
    desc.MiscFlags = uiOptionFlags;

    const HRESULT res = m_pD3D11Device->CreateTexture3D(&desc, nullptr, reinterpret_cast<ID3D11Texture3D**>(&m_pTexture));
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 texture: {}", (ezUInt32)HRESULT_CODE(res));
  }
  else
  {
    EZ_ASSERT_NOT_IMPLEMENTED
  }

  m_bIsResourceCreated = true;
}

#pragma endregion
