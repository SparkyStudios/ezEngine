#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/Shader.h>

#pragma region spShaderProgramD3D11

void spShaderProgramD3D11::ReleaseResource()
{
  DetachAll();
  m_bReleased = true;
}

bool spShaderProgramD3D11::IsReleased() const
{
  return m_bReleased;
}

void spShaderProgramD3D11::Attach(const spResourceHandle& hShader)
{
  auto* pShader = m_pDevice->GetResourceManager()->GetResource<spShaderD3D11>(hShader);
  EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader handle {0}", hShader);

  switch (pShader->GetStage())
  {
    case spShaderStage::VertexShader:
      m_pVertexShader = pShader;
      break;
    case spShaderStage::GeometryShader:
      m_pGeometryShader = pShader;
      break;
    case spShaderStage::HullShader:
      m_pHullShader = pShader;
      break;
    case spShaderStage::DomainShader:
      m_pDomainShader = pShader;
      break;
    case spShaderStage::PixelShader:
      m_pPixelShader = pShader;
      break;
    case spShaderStage::ComputeShader:
      m_pComputeShader = pShader;
      break;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return;
  }

  pShader->AddRef();
}

void spShaderProgramD3D11::Detach(const spResourceHandle& hShader)
{
  auto* pShader = m_pDevice->GetResourceManager()->GetResource<spShaderD3D11>(hShader);
  EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader handle {0}", hShader);

  spShaderD3D11* pFoundShader = nullptr;

  switch (pShader->GetStage())
  {
    case spShaderStage::VertexShader:
      if (m_pVertexShader == pShader)
      {
        pFoundShader = m_pVertexShader;
        m_pVertexShader = nullptr;
      }
      break;
    case spShaderStage::GeometryShader:
      if (m_pGeometryShader == pShader)
      {
        pFoundShader = m_pGeometryShader;
        m_pGeometryShader = nullptr;
      }
      break;
    case spShaderStage::HullShader:
      if (m_pHullShader == pShader)
      {
        pFoundShader = m_pHullShader;
        m_pHullShader = nullptr;
      }
      break;
    case spShaderStage::DomainShader:
      if (m_pDomainShader == pShader)
      {
        pFoundShader = m_pDomainShader;
        m_pDomainShader = nullptr;
      }
      break;
    case spShaderStage::PixelShader:
      if (m_pPixelShader == pShader)
      {
        pFoundShader = m_pPixelShader;
        m_pPixelShader = nullptr;
      }
      break;
    case spShaderStage::ComputeShader:
      if (m_pComputeShader == pShader)
      {
        pFoundShader = m_pComputeShader;
        m_pComputeShader = nullptr;
      }
      break;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  if (pFoundShader != nullptr)
    pFoundShader->ReleaseRef();
}

void spShaderProgramD3D11::Detach(const ezEnum<spShaderStage>& eStage)
{
  switch (eStage)
  {
    case spShaderStage::VertexShader:
      if (m_pVertexShader != nullptr)
      {
        m_pVertexShader->ReleaseRef();
        m_pVertexShader = nullptr;
      }
      break;
    case spShaderStage::GeometryShader:
      if (m_pGeometryShader != nullptr)
      {
        m_pGeometryShader->ReleaseRef();
        m_pGeometryShader = nullptr;
      }
      break;
    case spShaderStage::HullShader:
      if (m_pHullShader != nullptr)
      {
        m_pHullShader->ReleaseRef();
        m_pHullShader = nullptr;
      }
      break;
    case spShaderStage::DomainShader:
      if (m_pDomainShader != nullptr)
      {
        m_pDomainShader->ReleaseRef();
        m_pDomainShader = nullptr;
      }
      break;
    case spShaderStage::PixelShader:
      if (m_pPixelShader != nullptr)
      {
        m_pPixelShader->ReleaseRef();
        m_pPixelShader = nullptr;
      }
      break;
    case spShaderStage::ComputeShader:
      if (m_pComputeShader != nullptr)
      {
        m_pComputeShader->ReleaseRef();
        m_pComputeShader = nullptr;
      }
      break;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      break;
  }
}

void spShaderProgramD3D11::DetachAll()
{
  if (m_pVertexShader != nullptr)
    m_pVertexShader->ReleaseRef();

  if (m_pGeometryShader != nullptr)
    m_pGeometryShader->ReleaseRef();

  if (m_pHullShader != nullptr)
    m_pHullShader->ReleaseRef();

  if (m_pDomainShader != nullptr)
    m_pDomainShader->ReleaseRef();

  if (m_pPixelShader != nullptr)
    m_pPixelShader->ReleaseRef();

  if (m_pComputeShader != nullptr)
    m_pComputeShader->ReleaseRef();
}

void spShaderProgramD3D11::Use()
{
  // TODO
}

spResourceHandle spShaderProgramD3D11::Get(const ezEnum<spShaderStage>& eStage) const
{
  switch (eStage)
  {
    case spShaderStage::VertexShader:
      return m_pVertexShader->GetHandle();
    case spShaderStage::GeometryShader:
      return m_pGeometryShader->GetHandle();
    case spShaderStage::HullShader:
      return m_pHullShader->GetHandle();
    case spShaderStage::DomainShader:
      return m_pDomainShader->GetHandle();
    case spShaderStage::PixelShader:
      return m_pPixelShader->GetHandle();
    case spShaderStage::ComputeShader:
      return m_pComputeShader->GetHandle();
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return {};
  }
}

spShaderProgramD3D11::spShaderProgramD3D11(spDeviceD3D11* pDevice)
  : spShaderProgram()
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();

  m_bReleased = false;
}

spShaderProgramD3D11::~spShaderProgramD3D11()
{
  spShaderProgramD3D11::ReleaseResource();

  m_pVertexShader = nullptr;
  m_pGeometryShader = nullptr;
  m_pHullShader = nullptr;
  m_pDomainShader = nullptr;
  m_pPixelShader = nullptr;
  m_pComputeShader = nullptr;
}

#pragma endregion

#pragma region spShaderD3D11

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
      break;
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

#pragma endregion
