#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Sampler.h>

#pragma region spSamplerStateD3D11

spSamplerStateD3D11::spSamplerStateD3D11(spDeviceD3D11* pDevice, const spSamplerDescription& description)
  : m_Description(description)
{
  m_pDevice = pDevice;

  D3D11_SAMPLER_DESC desc;
  desc.AddressU = spToD3D11(description.m_eAddressModeR);
  desc.AddressV = spToD3D11(description.m_eAddressModeS);
  desc.AddressW = spToD3D11(description.m_eAddressModeT);
  desc.Filter = spToD3D11(description.m_eMinFilter, description.m_eMagFilter, description.m_eMipFilter, description.m_eSamplerComparison != spDepthStencilComparison::None);
  desc.MinLOD = static_cast<float>(description.m_uiMinLod);
  desc.MaxLOD = static_cast<float>(description.m_uiMaxLod);
  desc.MaxAnisotropy = description.m_uiMaxAnisotropy;
  desc.ComparisonFunc = spToD3D11(description.m_eSamplerComparison);
  desc.MipLODBias = description.m_fLodBias;
  desc.BorderColor[0] = description.m_BorderColor.r;
  desc.BorderColor[1] = description.m_BorderColor.g;
  desc.BorderColor[2] = description.m_BorderColor.b;
  desc.BorderColor[3] = description.m_BorderColor.a;

  const HRESULT res = pDevice->GetD3D11Device()->CreateSamplerState(&desc, &m_pSamplerState);
  EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 sampler state. Error Code: {}", (ezUInt32)HRESULT_CODE(res));
}

spSamplerStateD3D11::~spSamplerStateD3D11()
{
  SP_RHI_DX11_RELEASE(m_pSamplerState);
}

spSamplerDescription spSamplerStateD3D11::GetSamplerDescription() const
{
  return m_Description;
}

void spSamplerStateD3D11::SetDebugName(const ezString& sDebugName)
{
  spDeviceResource::SetDebugName(sDebugName);
  m_pSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, sDebugName.GetElementCount(), sDebugName.GetData());
}

void spSamplerStateD3D11::ReleaseResource()
{
  SP_RHI_DX11_RELEASE(m_pSamplerState);
}

bool spSamplerStateD3D11::IsReleased() const
{
  return m_pSamplerState == nullptr;
}

#pragma endregion

#pragma region spSamplerD3D11

spResourceHandle spSamplerD3D11::GetSamplerWithMipMap() const
{
  return m_pSamplerState->GetHandle();
}

spResourceHandle spSamplerD3D11::GetSamplerWithoutMipMap() const
{
  return m_pSamplerState->GetHandle();
}

void spSamplerD3D11::CreateResource()
{
  if (!IsReleased())
    return;

  m_pSamplerState = new spSamplerStateD3D11(ezStaticCast<spDeviceD3D11*>(m_pDevice), m_Description);
  m_pDevice->GetResourceManager()->RegisterResource(m_pSamplerState);

  m_bIsResourceCreated = true;
}

void spSamplerD3D11::SetDebugName(const ezString& name)
{
  spDeviceResource::SetDebugName(name);

  m_pSamplerState->m_pSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, name.GetElementCount(), name.GetData());
}

void spSamplerD3D11::ReleaseResource()
{
  m_pDevice->GetResourceManager()->ReleaseResource(m_pSamplerState->GetHandle());

  m_bIsResourceCreated = false;
}

bool spSamplerD3D11::IsReleased() const
{
  return m_pSamplerState->IsReleased();
}

spSamplerD3D11::spSamplerD3D11(spDeviceD3D11* pDevice, const spSamplerDescription& description)
  : m_pSamplerState(nullptr)
{
  m_pDevice = pDevice;
  m_Description = description;
}

#pragma endregion
