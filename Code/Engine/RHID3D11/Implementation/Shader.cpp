#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/Shader.h>

void spShaderD3D11::ReleaseResource()
{
  SP_RHI_DX11_RELEASE(m_pD3D11Shader);
  m_bIsResourceCreated = false;
}

bool spShaderD3D11::IsReleased() const
{
  return m_pD3D11Shader == nullptr;
}

void spShaderD3D11::SetDebugName(const ezString& debugName)
{
  spShader::SetDebugName(debugName);
}

void spShaderD3D11::CreateResource()
{
  // If the description contains a compiled shader binary (will be the case most of the time)
  if (m_Description.m_Buffer.GetCount() > 4 && m_Description.m_Buffer[0] == 0x44 && m_Description.m_Buffer[1] == 0x58 && m_Description.m_Buffer[2] == 0x42 && m_Description.m_Buffer[3] == 0x43)
    m_pByteCode.CopyFrom(m_Description.m_Buffer);
  else
    EZ_ASSERT_NOT_IMPLEMENTED; // Not yet implemented shader compiling, maybe a chance to add SPSL as the default shading language ?

  switch (m_Description.m_eShaderStage)
  {
    case spShaderStage::VertexShader:
    {
      const HRESULT res = m_pD3D11Device->CreateVertexShader(m_pByteCode.GetPtr(), m_pByteCode.GetCount(), nullptr, reinterpret_cast<ID3D11VertexShader**>(&m_pD3D11Shader));
      EZ_HRESULT_TO_ASSERT(res);
      break;
    }
    case spShaderStage::HullShader:
    {
      const HRESULT res = m_pD3D11Device->CreateHullShader(m_pByteCode.GetPtr(), m_pByteCode.GetCount(), nullptr, reinterpret_cast<ID3D11HullShader**>(&m_pD3D11Shader));
      EZ_HRESULT_TO_ASSERT(res);
      break;
    }
    case spShaderStage::DomainShader:
    {
      const HRESULT res = m_pD3D11Device->CreateDomainShader(m_pByteCode.GetPtr(), m_pByteCode.GetCount(), nullptr, reinterpret_cast<ID3D11DomainShader**>(&m_pD3D11Shader));
      EZ_HRESULT_TO_ASSERT(res);
      break;
    }
    case spShaderStage::GeometryShader:
    {
      const HRESULT res = m_pD3D11Device->CreateGeometryShader(m_pByteCode.GetPtr(), m_pByteCode.GetCount(), nullptr, reinterpret_cast<ID3D11GeometryShader**>(&m_pD3D11Shader));
      EZ_HRESULT_TO_ASSERT(res);
      break;
    }
    case spShaderStage::PixelShader:
    {
      const HRESULT res = m_pD3D11Device->CreatePixelShader(m_pByteCode.GetPtr(), m_pByteCode.GetCount(), nullptr, reinterpret_cast<ID3D11PixelShader**>(&m_pD3D11Shader));
      EZ_HRESULT_TO_ASSERT(res);
      break;
    }
    case spShaderStage::ComputeShader:
    {
      const HRESULT res = m_pD3D11Device->CreateComputeShader(m_pByteCode.GetPtr(), m_pByteCode.GetCount(), nullptr, reinterpret_cast<ID3D11ComputeShader**>(&m_pD3D11Shader));
      EZ_HRESULT_TO_ASSERT(res);
      break;
    }
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
  }

  m_bIsResourceCreated = true;
}

spShaderD3D11::spShaderD3D11(spDeviceD3D11* pDevice, const spShaderDescription& description)
  : spShader(description)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();
}

spShaderD3D11::~spShaderD3D11()
{
  spShaderD3D11::ReleaseResource();
}
