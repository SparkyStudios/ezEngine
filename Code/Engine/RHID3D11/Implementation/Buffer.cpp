#include <RHID3D11/RHID3D11PCH.h>

#include <dxgiformat.h>

#include <RHID3D11/Buffer.h>
#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Fence.h>

#pragma region spBufferRangeD3D11

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spBufferRangeD3D11, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void spBufferRangeD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  m_pBuffer = nullptr;
  m_pFence.Clear();

  m_bReleased = true;
}

spBufferRangeD3D11::spBufferRangeD3D11(spDeviceD3D11* pDevice, const spBufferRangeDescription& description)
  : spBufferRange(description)
{
  m_pDevice = pDevice;

  // Need to borrow the pointer to not create a new reference
  m_pBuffer = pDevice->GetResourceManager()->GetResource<spBufferD3D11>(description.m_hBuffer);
  EZ_ASSERT_DEV(m_pBuffer != nullptr, "Buffer range creation failed. Invalid parent buffer provided.");

  m_pFence = m_pDevice->GetResourceFactory()->CreateFence(description.m_Fence).Downcast<spFenceD3D11>();

  m_bReleased = false;
}

spBufferRangeD3D11::~spBufferRangeD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

#pragma endregion

#pragma region spBufferD3D11

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spBufferD3D11, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void spBufferD3D11::SetDebugName(ezStringView sDebugName)
{
  spBuffer::SetDebugName(sDebugName);

  if (IsReleased())
    return;

  ezStringBuilder srvName(sDebugName);
  ezStringBuilder uavName(sDebugName);

  srvName.Append("_SRV");
  uavName.Append("_UAV");

  for (auto& buffer : m_pShaderResourceViews)
    buffer.Value()->SetPrivateData(WKPDID_D3DDebugObjectName, srvName.GetElementCount(), srvName.GetData());

  for (auto& buffer : m_pUnorderedAccessViews)
    buffer.Value()->SetPrivateData(WKPDID_D3DDebugObjectName, uavName.GetElementCount(), uavName.GetData());

  m_pBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sDebugName.GetElementCount(), sDebugName.GetStartPointer());
}

void spBufferD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  for (auto& range : m_BufferRanges)
    range.Clear();

  for (auto& buffer : m_pShaderResourceViews)
    SP_RHI_DX11_RELEASE(buffer.Value());

  for (auto& buffer : m_pUnorderedAccessViews)
    SP_RHI_DX11_RELEASE(buffer.Value());

  SP_RHI_DX11_RELEASE(m_pBuffer);

  m_pShaderResourceViews.Clear();
  m_pUnorderedAccessViews.Clear();

  m_BufferRanges.Clear();

  m_bIsResourceCreated = false;
}

void spBufferD3D11::CreateResource()
{
  PreCreateResource();

  D3D11_BUFFER_DESC desc;
  desc.BindFlags = spToD3D11(m_Description.m_eUsage);
  desc.ByteWidth = m_uiBufferAlignedSize * m_uiBufferCount;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  if (m_Description.m_eUsage.IsAnySet(spBufferUsage::StructuredBufferReadOnly | spBufferUsage::StructuredBufferReadWrite))
  {
    if (m_Description.m_bRawBuffer)
    {
      desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    }
    else
    {
      desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
      desc.StructureByteStride = m_Description.m_uiStructureStride;
    }
  }

  if (m_Description.m_eUsage.IsSet(spBufferUsage::IndirectBuffer))
    desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

  if (m_Description.m_eUsage.IsSet(spBufferUsage::Dynamic))
  {
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  }
  else if (m_Description.m_eUsage.IsSet(spBufferUsage::Staging))
  {
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  }

  const HRESULT res = m_pD3D11Device->CreateBuffer(&desc, nullptr, &m_pBuffer);
  EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 buffer. Error Code: {}.", (ezUInt32)HRESULT_CODE(res));

  PostCreateResource();

  SetDebugName(m_sDebugName);

  m_bIsResourceCreated = true;
}

spBufferD3D11::spBufferD3D11(spDeviceD3D11* pDevice, const spBufferDescription& description)
  : spBuffer(description)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();
}

spBufferD3D11::~spBufferD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

ID3D11ShaderResourceView* spBufferD3D11::GetShaderResourceView(ezUInt32 uiOffset, ezUInt32 uiSize)
{
  EZ_LOCK(m_AccessViewLock);

  OffsetSizePair pair(uiOffset, uiSize);
  ID3D11ShaderResourceView* pSRV;

  if (!m_pShaderResourceViews.TryGetValue(pair, pSRV))
  {
    pSRV = CreateShaderResourceView(uiOffset, uiSize);
    m_pShaderResourceViews.Insert(pair, pSRV);
  }

  return pSRV;
}

ID3D11UnorderedAccessView* spBufferD3D11::GetUnorderedAccessView(ezUInt32 uiOffset, ezUInt32 uiSize)
{
  EZ_LOCK(m_AccessViewLock);

  OffsetSizePair pair(uiOffset, uiSize);
  ID3D11UnorderedAccessView* pUAV;

  if (!m_pUnorderedAccessViews.TryGetValue(pair, pUAV))
  {
    pUAV = CreateUnorderedAccessView(uiOffset, uiSize);
    m_pUnorderedAccessViews.Insert(pair, pUAV);
  }

  return pUAV;
}

ID3D11ShaderResourceView* spBufferD3D11::CreateShaderResourceView(ezUInt32 uiOffset, ezUInt32 uiSize) const
{
  D3D11_SHADER_RESOURCE_VIEW_DESC desc;
  ID3D11ShaderResourceView* pSRV = nullptr;

  if (IsRawBuffer())
  {
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BufferEx.FirstElement = uiOffset / 4;
    desc.BufferEx.NumElements = uiSize / 4;
    desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
  }
  else
  {
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.NumElements = (uiSize / m_Description.m_uiStructureStride);
    desc.Buffer.ElementOffset = (uiOffset / m_Description.m_uiStructureStride);
  }

  const HRESULT res = m_pD3D11Device->CreateShaderResourceView(m_pBuffer, &desc, &pSRV);
  EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 shader resource view for a buffer resource. Error Code: {}", (ezUInt32)HRESULT_CODE(res));

  return pSRV;
}

ID3D11UnorderedAccessView* spBufferD3D11::CreateUnorderedAccessView(ezUInt32 uiOffset, ezUInt32 uiSize) const
{
  D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
  ID3D11UnorderedAccessView* pUAV = nullptr;

  if (IsRawBuffer())
  {
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = uiOffset / 4;
    desc.Buffer.NumElements = uiSize / 4;
    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
  }
  else
  {
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = (uiOffset / m_Description.m_uiStructureStride);
    desc.Buffer.NumElements = (uiSize / m_Description.m_uiStructureStride);
  }

  const HRESULT res = m_pD3D11Device->CreateUnorderedAccessView(m_pBuffer, &desc, &pUAV);
  EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 unordered access view for a buffer resource. Error Code: {}", (ezUInt32)HRESULT_CODE(res));

  return pUAV;
}

#pragma endregion

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Buffer);
