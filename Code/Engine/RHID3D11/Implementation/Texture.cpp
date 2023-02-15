#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Texture.h>

static D3D11_SHADER_RESOURCE_VIEW_DESC spGetShaderResourceViewDesc(spTextureD3D11* pTexture, ezUInt32 uiBaseMipLevel, ezUInt32 uiMipCount, ezUInt32 uiBaseArrayLayer, ezUInt32 uiArrayLayerCount, const ezEnum<spPixelFormat>& eFormat)
{
  D3D11_SHADER_RESOURCE_VIEW_DESC desc;
  desc.Format = spGetViewFormat(spToD3D11(eFormat, pTexture->GetUsage().IsSet(spTextureUsage::DepthStencil)));

  if (pTexture->GetUsage().IsSet(spTextureUsage::Cubemap))
  {
    desc.TextureCube.MipLevels = uiMipCount;
    desc.TextureCube.MostDetailedMip = uiBaseMipLevel;

    if (pTexture->GetArrayLayerCount() == 1)
    {
      desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    }
    else
    {
      desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
      desc.TextureCubeArray.First2DArrayFace = uiBaseArrayLayer;
      desc.TextureCubeArray.NumCubes = uiArrayLayerCount;
    }
  }
  else if (pTexture->GetDepth() == 1)
  {
    if (pTexture->GetArrayLayerCount() == 1)
    {
      if (pTexture->GetDimension() == spTextureDimension::Texture1D)
      {
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        desc.Texture1D.MipLevels = uiMipCount;
        desc.Texture1D.MostDetailedMip = uiBaseMipLevel;
      }
      else
      {
        desc.ViewDimension = pTexture->GetSampleCount() == spTextureSampleCount::None ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
        desc.Texture2D.MipLevels = uiMipCount;
        desc.Texture2D.MostDetailedMip = uiBaseMipLevel;
      }
    }
    else
    {
      if (pTexture->GetDimension() == spTextureDimension::Texture1D)
      {
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
        desc.Texture1DArray.MipLevels = uiMipCount;
        desc.Texture1DArray.MostDetailedMip = uiBaseMipLevel;
        desc.Texture1DArray.FirstArraySlice = uiBaseArrayLayer;
        desc.Texture1DArray.ArraySize = uiArrayLayerCount;
      }
      else
      {
        desc.ViewDimension = pTexture->GetSampleCount() == spTextureSampleCount::None ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
        desc.Texture2DArray.MipLevels = uiMipCount;
        desc.Texture2DArray.MostDetailedMip = uiBaseMipLevel;
        desc.Texture2DArray.FirstArraySlice = uiBaseArrayLayer;
        desc.Texture2DArray.ArraySize = uiArrayLayerCount;
      }
    }
  }
  else
  {
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MipLevels = uiMipCount;
    desc.Texture3D.MostDetailedMip = uiBaseMipLevel;
  }

  return desc;
}

#pragma region spTextureD3D11

spTextureD3D11::spTextureD3D11(spDeviceD3D11* pDevice, const spTextureDescription& description)
  : spTexture(description)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();

  m_eFormat = spToD3D11(description.m_eFormat, description.m_eUsage.IsSet(spTextureUsage::DepthStencil));
  m_eTypelessFormat = spGetTypelessFormat(m_eFormat);
}

spTextureD3D11::~spTextureD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

void spTextureD3D11::SetDebugName(const ezString& debugName)
{
  spDeviceResource::SetDebugName(debugName);

  m_pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.GetElementCount(), debugName.GetData());
}

void spTextureD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  if (m_pParentTexture != nullptr)
    m_pParentTexture.Clear();
  else if (m_bFromNative)
    m_pTexture->Release();

  m_pTexture = nullptr;
  m_pParentTexture = nullptr;

  m_bIsResourceCreated = false;
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
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 texture. Error Code: {}", (ezUInt32)HRESULT_CODE(res));
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
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 texture. Error Code: {}", (ezUInt32)HRESULT_CODE(res));
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
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 texture. Error Code: {}", (ezUInt32)HRESULT_CODE(res));
  }
  else
  {
    EZ_ASSERT_NOT_IMPLEMENTED
  }

  m_bIsResourceCreated = true;
}

ezSharedPtr<spTextureD3D11> spTextureD3D11::FromExisting(const ezSharedPtr<spTextureD3D11>& pTexture)
{
  spTextureDescription desc;
  desc.m_uiWidth = pTexture->GetWidth();
  desc.m_uiHeight = pTexture->GetHeight();
  desc.m_uiDepth = pTexture->GetDepth();
  desc.m_uiMipCount = pTexture->GetMipCount();
  desc.m_uiArrayLayers = pTexture->GetArrayLayerCount();
  desc.m_eFormat = pTexture->GetFormat();
  desc.m_eSampleCount = pTexture->GetSampleCount();
  desc.m_eDimension = pTexture->GetDimension();
  desc.m_eUsage = pTexture->GetUsage();

  auto pResult = pTexture->GetDevice()->GetResourceFactory()->CreateTexture(desc).Downcast<spTextureD3D11>();
  pResult->m_pDevice = pTexture->GetDevice();
  pResult->m_pD3D11Device = ezStaticCast<spDeviceD3D11*>(pTexture->GetDevice())->GetD3D11Device();
  pResult->m_pTexture = pTexture->GetD3D11Texture();
  pResult->m_eFormat = pTexture->m_eFormat;
  pResult->m_eTypelessFormat = pTexture->m_eTypelessFormat;
  pResult->m_pParentTexture = pTexture;
  pResult->m_bIsResourceCreated = true;

  EZ_IGNORE_UNUSED(pTexture->AddRef());

  return pResult;
}

ezSharedPtr<spTextureD3D11> spTextureD3D11::FromNative(spDeviceD3D11* pDevice, ID3D11Texture2D* pTexture, const ezEnum<spTextureDimension>& eDimension, const ezEnum<spPixelFormat>& eFormat)
{
  D3D11_TEXTURE2D_DESC nativeDesc;
  pTexture->GetDesc(&nativeDesc);

  spTextureDescription desc;
  desc.m_uiWidth = nativeDesc.Width;
  desc.m_uiHeight = nativeDesc.Height;
  desc.m_uiDepth = 1;
  desc.m_uiMipCount = nativeDesc.MipLevels;
  desc.m_uiArrayLayers = nativeDesc.ArraySize;
  desc.m_eFormat = eFormat;
  desc.m_eSampleCount = spTextureSampleCount::GetSampleCount(static_cast<spTextureSampleCount::StorageType>(nativeDesc.SampleDesc.Count));
  desc.m_eDimension = eDimension;
  desc.m_eUsage = spGetTextureUsage(nativeDesc.BindFlags, nativeDesc.CPUAccessFlags, nativeDesc.MiscFlags);

  auto pResult = pDevice->GetResourceFactory()->CreateTexture(desc).Downcast<spTextureD3D11>();
  pResult->m_pDevice = pDevice;
  pResult->m_pD3D11Device = ezStaticCast<spDeviceD3D11*>(pDevice)->GetD3D11Device();
  pResult->m_pTexture = pTexture;
  pResult->m_eFormat = spToD3D11(eFormat, desc.m_eUsage.IsSet(spTextureUsage::DepthStencil));
  pResult->m_eTypelessFormat = spGetTypelessFormat(pResult->m_eFormat);
  pResult->m_bFromNative = true;
  pResult->m_bIsResourceCreated = true;

  pTexture->AddRef();

  return pResult;
}

ID3D11Resource* spTextureD3D11::GetD3D11Texture() const
{
  return m_pTexture;
}

#pragma endregion

#pragma region spTextureViewD3D11

void spTextureViewD3D11::SetDebugName(const ezString& debugName)
{
  spTextureView::SetDebugName(debugName);

  ezStringBuilder sSRVDebugName(debugName);
  sSRVDebugName.Append("_SRV");

  ezStringBuilder sUAVDebugName(debugName);
  sUAVDebugName.Append("_UAV");

  m_pShaderResourceView->SetPrivateData(WKPDID_D3DDebugObjectName, sSRVDebugName.GetElementCount(), sSRVDebugName.GetData());
  m_pUnorderedAccessView->SetPrivateData(WKPDID_D3DDebugObjectName, sUAVDebugName.GetElementCount(), sUAVDebugName.GetData());
}

void spTextureViewD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  SP_RHI_DX11_RELEASE(m_pShaderResourceView);
  SP_RHI_DX11_RELEASE(m_pUnorderedAccessView);

  m_bIsResourceCreated = false;
}

bool spTextureViewD3D11::IsReleased() const
{
  return m_pShaderResourceView == nullptr && m_pUnorderedAccessView == nullptr;
}

void spTextureViewD3D11::CreateResource()
{
  auto pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(m_Description.m_hTarget);
  EZ_ASSERT_DEV(pTexture != nullptr, "Texture view resource using invalid texture resource as a target.");

  // SRV
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC desc = spGetShaderResourceViewDesc(pTexture, m_Description.m_uiBaseMipLevel, m_Description.m_uiMipCount, m_Description.m_uiBaseArrayLayer, m_Description.m_uiArrayLayers, m_Description.m_eFormat);

    const HRESULT res = m_pD3D11Device->CreateShaderResourceView(pTexture->GetD3D11Texture(), &desc, &m_pShaderResourceView);
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 shader resource view for a texture view resource. Error Code: {}", (ezUInt32)HRESULT_CODE(res));
  }

  // UAV
  if (pTexture->GetUsage().IsSet(spTextureUsage::Storage))
  {
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    desc.Format = spGetViewFormat(pTexture->m_eFormat);

    if (pTexture->GetUsage().IsSet(spTextureUsage::Cubemap))
    {
      EZ_ASSERT_NOT_IMPLEMENTED
    }
    else if (pTexture->GetDepth() == 1)
    {
      if (pTexture->GetArrayLayerCount() == 1)
      {
        if (pTexture->GetDimension() == spTextureDimension::Texture1D)
        {
          desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
          desc.Texture1D.MipSlice = m_Description.m_uiBaseMipLevel;
        }
        else
        {
          desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
          desc.Texture2D.MipSlice = m_Description.m_uiBaseMipLevel;
        }
      }
      else
      {
        if (pTexture->GetDimension() == spTextureDimension::Texture1D)
        {
          desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
          desc.Texture1DArray.MipSlice = m_Description.m_uiBaseMipLevel;
          desc.Texture1DArray.FirstArraySlice = m_Description.m_uiBaseArrayLayer;
          desc.Texture1DArray.ArraySize = m_Description.m_uiArrayLayers;
        }
        else
        {
          desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
          desc.Texture2DArray.MipSlice = m_Description.m_uiBaseMipLevel;
          desc.Texture2DArray.FirstArraySlice = m_Description.m_uiBaseArrayLayer;
          desc.Texture2DArray.ArraySize = m_Description.m_uiArrayLayers;
        }
      }
    }
    else
    {
      desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
      desc.Texture3D.MipSlice = m_Description.m_uiBaseMipLevel;

      // Maps the entire range of the 3D texture
      desc.Texture3D.FirstWSlice = 0;
      desc.Texture3D.WSize = pTexture->GetDepth();
    }

    const HRESULT res = m_pD3D11Device->CreateUnorderedAccessView(pTexture->GetD3D11Texture(), &desc, &m_pUnorderedAccessView);
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 unordered access view for a texture view resource. Error Code: {}", (ezUInt32)HRESULT_CODE(res));
  }

  m_bIsResourceCreated = true;
}

spTextureViewD3D11::spTextureViewD3D11(spDeviceD3D11* pDevice, const spTextureViewDescription& description)
  : spTextureView(description)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();
}

spTextureViewD3D11::~spTextureViewD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

ID3D11ShaderResourceView* spTextureViewD3D11::GetShaderResourceView() const
{
  return m_pShaderResourceView;
}

ID3D11UnorderedAccessView* spTextureViewD3D11::GetUnorderedAccessView() const
{
  return m_pUnorderedAccessView;
}

#pragma endregion

#pragma region spTextureSamplerManagerD3D11

ezSharedPtr<spTextureView> spTextureSamplerManagerD3D11::GetFullTextureView(ezSharedPtr<spTexture> pTexture)
{
  const auto* pDevice = ezSingletonRegistry::GetSingletonInstance<spDevice>();
  return pDevice->GetResourceFactory()->CreateTextureView(pTexture->GetHandle());
}

spTextureSamplerManagerD3D11::spTextureSamplerManagerD3D11(spDeviceD3D11* pDevice)
  : spTextureSamplerManager(pDevice)
{
}

#pragma endregion

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Texture);
