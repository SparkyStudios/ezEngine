#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/Shader.h>

#include <d3dcommon.h>
#include <d3dcompiler.h>

namespace RHI
{
#pragma region spShaderProgramD3D11

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderProgramD3D11, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spShaderProgramD3D11::ReleaseResource()
  {
    if (IsReleased())
      return;

    DetachAll();
    m_bReleased = true;
  }

  bool spShaderProgramD3D11::IsReleased() const
  {
    return m_bReleased;
  }

  void spShaderProgramD3D11::Attach(ezSharedPtr<spShader> pShader)
  {
    EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader handle.");

    switch (pShader->GetStage())
    {
      case spShaderStage::VertexShader:
        m_pVertexShader = pShader.Downcast<spShaderD3D11>();
        break;
      case spShaderStage::HullShader:
        m_pHullShader = pShader.Downcast<spShaderD3D11>();
        break;
      case spShaderStage::DomainShader:
        m_pDomainShader = pShader.Downcast<spShaderD3D11>();
        break;
      case spShaderStage::GeometryShader:
        m_pGeometryShader = pShader.Downcast<spShaderD3D11>();
        break;
      case spShaderStage::PixelShader:
        m_pPixelShader = pShader.Downcast<spShaderD3D11>();
        break;
      case spShaderStage::ComputeShader:
        m_pComputeShader = pShader.Downcast<spShaderD3D11>();
        break;
      case spShaderStage::None:
      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
    }
  }

  void spShaderProgramD3D11::Detach(ezSharedPtr<spShader> pShader)
  {
    EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader handle.");

    Detach(pShader->GetStage());
  }

  void spShaderProgramD3D11::Detach(const ezEnum<spShaderStage>& eStage)
  {
    switch (eStage)
    {
      case spShaderStage::VertexShader:
        m_pVertexShader.Clear();
        break;
      case spShaderStage::HullShader:
        m_pHullShader.Clear();
        break;
      case spShaderStage::DomainShader:
        m_pDomainShader.Clear();
        break;
      case spShaderStage::GeometryShader:
        m_pGeometryShader.Clear();
        break;
      case spShaderStage::PixelShader:
        m_pPixelShader.Clear();
        break;
      case spShaderStage::ComputeShader:
        m_pComputeShader.Clear();
        break;
      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }

  void spShaderProgramD3D11::DetachAll()
  {
    m_pVertexShader.Clear();
    m_pHullShader.Clear();
    m_pDomainShader.Clear();
    m_pGeometryShader.Clear();
    m_pPixelShader.Clear();
    m_pComputeShader.Clear();
  }

  void spShaderProgramD3D11::Use()
  {
    // noop
  }

  ezSharedPtr<spShader> spShaderProgramD3D11::Get(const ezEnum<spShaderStage>& eStage) const
  {
    switch (eStage)
    {
      case spShaderStage::VertexShader:
        return m_pVertexShader;
      case spShaderStage::HullShader:
        return m_pHullShader;
      case spShaderStage::DomainShader:
        return m_pDomainShader;
      case spShaderStage::GeometryShader:
        return m_pGeometryShader;
      case spShaderStage::PixelShader:
        return m_pPixelShader;
      case spShaderStage::ComputeShader:
        return m_pComputeShader;
      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return nullptr;
    }
  }

  void spShaderProgramD3D11::GetResourceLayoutDescriptions(ezDynamicArray<spResourceLayoutDescription>& out_resourceLayouts) const
  {
    out_resourceLayouts.Clear();
    auto& resourceLayoutDescription = out_resourceLayouts.ExpandAndGetRef();

    if (m_pVertexShader != nullptr)
      GetResourceLayoutElementsForStage(spShaderStage::VertexShader, resourceLayoutDescription.m_Elements);
    if (m_pHullShader != nullptr)
      GetResourceLayoutElementsForStage(spShaderStage::HullShader, resourceLayoutDescription.m_Elements);
    if (m_pDomainShader != nullptr)
      GetResourceLayoutElementsForStage(spShaderStage::DomainShader, resourceLayoutDescription.m_Elements);
    if (m_pGeometryShader != nullptr)
      GetResourceLayoutElementsForStage(spShaderStage::GeometryShader, resourceLayoutDescription.m_Elements);
    if (m_pPixelShader != nullptr)
      GetResourceLayoutElementsForStage(spShaderStage::PixelShader, resourceLayoutDescription.m_Elements);
    if (m_pComputeShader != nullptr)
      GetResourceLayoutElementsForStage(spShaderStage::ComputeShader, resourceLayoutDescription.m_Elements);
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
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  void spShaderProgramD3D11::GetResourceLayoutElementsForStage(const ezEnum<RHI::spShaderStage>& eStage, ezDynamicArray<spResourceLayoutElementDescription>& out_elements) const
  {
    ezByteArrayPtr pByteCode;

    if (eStage == spShaderStage::VertexShader)
    {
      m_pVertexShader->EnsureResourceCreated();
      pByteCode = m_pVertexShader->GetShaderByteCode();
    }
    else if (eStage == spShaderStage::HullShader)
    {
      m_pHullShader->EnsureResourceCreated();
      pByteCode = m_pHullShader->GetShaderByteCode();
    }
    else if (eStage == spShaderStage::DomainShader)
    {
      m_pDomainShader->EnsureResourceCreated();
      pByteCode = m_pDomainShader->GetShaderByteCode();
    }
    else if (eStage == spShaderStage::GeometryShader)
    {
      m_pGeometryShader->EnsureResourceCreated();
      pByteCode = m_pGeometryShader->GetShaderByteCode();
    }
    else if (eStage == spShaderStage::PixelShader)
    {
      m_pPixelShader->EnsureResourceCreated();
      pByteCode = m_pPixelShader->GetShaderByteCode();
    }

    EZ_ASSERT_DEV(!pByteCode.IsEmpty(), "Unable to find shader byte code for stage {}.", eStage);

    ID3D11ShaderReflection* pReflector = nullptr;
    D3DReflect(pByteCode.GetPtr(), pByteCode.GetCount(), IID_ID3D11ShaderReflection, (void**)&pReflector);

    D3D11_SHADER_DESC shaderDesc;
    pReflector->GetDesc(&shaderDesc);

    for (ezUInt32 i = 0, l = shaderDesc.BoundResources; i < l; ++i)
    {
      D3D11_SHADER_INPUT_BIND_DESC shaderInputBindDesc;
      pReflector->GetResourceBindingDesc(i, &shaderInputBindDesc);

      auto& element = out_elements.ExpandAndGetRef();
      element.m_eType = spFromD3D11(shaderInputBindDesc.Type, shaderInputBindDesc.Dimension);
      element.m_eShaderStage = eStage;
      element.m_sName.Assign(shaderInputBindDesc.Name);
    }
  }

#pragma endregion

#pragma region spShaderD3D11

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderD3D11, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spShaderD3D11::ReleaseResource()
  {
    if (IsReleased())
      return;

    if (!m_pByteCode.IsEmpty())
      EZ_DEFAULT_DELETE_ARRAY(m_pByteCode);

    if (m_Description.m_bOwnBuffer)
      EZ_DEFAULT_DELETE_ARRAY(m_Description.m_Buffer);

    SP_RHI_DX11_RELEASE(m_pD3D11Shader);
    m_bIsResourceCreated = false;
  }

  bool spShaderD3D11::IsReleased() const
  {
    return m_pD3D11Shader == nullptr;
  }

  void spShaderD3D11::SetDebugName(ezStringView sDebugName)
  {
    spShader::SetDebugName(sDebugName);

    if (IsReleased())
      return;

    m_pD3D11Shader->SetPrivateData(WKPDID_D3DDebugObjectName, sDebugName.GetElementCount(), sDebugName.GetStartPointer());
  }

  void spShaderD3D11::CreateResource()
  {
    // If the description contains a compiled shader binary (will be the case most of the time)
    if (m_Description.m_Buffer.GetCount() > 4 && m_Description.m_Buffer[0] == 0x44 && m_Description.m_Buffer[1] == 0x58 && m_Description.m_Buffer[2] == 0x42 && m_Description.m_Buffer[3] == 0x43)
    {
      m_pByteCode = EZ_DEFAULT_NEW_ARRAY(ezUInt8, m_Description.m_Buffer.GetCount());
      m_pByteCode.CopyFrom(m_Description.m_Buffer);
    }
    else
    {
      const char* szProfile = nullptr;

      switch (m_Description.m_eShaderStage)
      {
        case spShaderStage::VertexShader:
          szProfile = "vs_5_0";
          break;
        case spShaderStage::HullShader:
          szProfile = "hs_5_0";
          break;
        case spShaderStage::DomainShader:
          szProfile = "ds_5_0";
          break;
        case spShaderStage::GeometryShader:
          szProfile = "gs_5_0";
          break;
        case spShaderStage::PixelShader:
          szProfile = "ps_5_0";
          break;
        case spShaderStage::ComputeShader:
          szProfile = "cs_5_0";
          break;
        case spShaderStage::None:
        default:
          EZ_ASSERT_NOT_IMPLEMENTED;
          break;
      }

      ID3DBlob* pResultBlob = nullptr;
      ID3DBlob* pErrorBlob = nullptr;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
      UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ENABLE_STRICTNESS;
#else
      UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

      if (SUCCEEDED(D3DCompile(m_Description.m_Buffer.GetPtr(), m_Description.m_Buffer.GetCount(), nullptr, nullptr, nullptr, m_Description.m_sEntryPoint.GetData(), szProfile, flags, 0, &pResultBlob, &pErrorBlob)))
      {
        if (pResultBlob != nullptr)
        {
          const auto uiByteCodeSize = static_cast<ezUInt32>(pResultBlob->GetBufferSize());
          m_pByteCode = EZ_DEFAULT_NEW_ARRAY(ezUInt8, uiByteCodeSize);
          m_pByteCode.CopyFrom(ezMakeArrayPtr(static_cast<ezUInt8*>(pResultBlob->GetBufferPointer()), uiByteCodeSize));
          pResultBlob->Release();
        }
      }

      if (pErrorBlob != nullptr)
      {
        const char* szError = static_cast<const char*>(pErrorBlob->GetBufferPointer());

        EZ_LOG_BLOCK("Shader Compilation Error Message");
        ezLog::Dev("{0}", szError);

        pErrorBlob->Release();
      }
    } // Not yet implemented shader compiling, maybe a chance to add SPSL as the default shading language ?

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

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    SetDebugName(m_sDebugName);
#endif

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
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Shader);
