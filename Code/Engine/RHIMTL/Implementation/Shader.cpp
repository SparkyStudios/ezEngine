#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Core.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Shader.h>

namespace RHI
{
#pragma region spShaderProgramMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderProgramMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spShaderProgramMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    DetachAll();
    m_bReleased = true;
  }

  bool spShaderProgramMTL::IsReleased() const
  {
    return m_bReleased;
  }

  void spShaderProgramMTL::Attach(ezSharedPtr<spShader> pShader)
  {
    EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader handle.");

    switch (pShader->GetStage())
    {
      case spShaderStage::VertexShader:
        m_pVertexShader = pShader.Downcast<spShaderMTL>();
        break;
      case spShaderStage::HullShader:
        m_pHullShader = pShader.Downcast<spShaderMTL>();
        break;
      case spShaderStage::DomainShader:
        m_pDomainShader = pShader.Downcast<spShaderMTL>();
        break;
      case spShaderStage::GeometryShader:
        m_pGeometryShader = pShader.Downcast<spShaderMTL>();
        break;
      case spShaderStage::PixelShader:
        m_pPixelShader = pShader.Downcast<spShaderMTL>();
        break;
      case spShaderStage::ComputeShader:
        m_pComputeShader = pShader.Downcast<spShaderMTL>();
        break;
      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }

  void spShaderProgramMTL::Detach(ezSharedPtr<spShader> pShader)
  {
    EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader handle.");

    Detach(pShader->GetStage());
  }

  void spShaderProgramMTL::Detach(const ezEnum<spShaderStage>& eStage)
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

  void spShaderProgramMTL::DetachAll()
  {
    m_pVertexShader.Clear();
    m_pHullShader.Clear();
    m_pDomainShader.Clear();
    m_pGeometryShader.Clear();
    m_pPixelShader.Clear();
    m_pComputeShader.Clear();
  }

  ezSharedPtr<spShader> spShaderProgramMTL::Get(const ezEnum<spShaderStage>& eStage) const
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

  void spShaderProgramMTL::GetResourceLayoutDescriptions(ezDynamicArray<spResourceLayoutDescription>& out_resourceLayouts) const
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

  spShaderProgramMTL::spShaderProgramMTL(spDeviceMTL* pDevice)
    : spShaderProgram()
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();

    m_bReleased = false;
  }

  spShaderProgramMTL::~spShaderProgramMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  void spShaderProgramMTL::GetResourceLayoutElementsForStage(const ezEnum<spShaderStage>& eStage, ezDynamicArray<spResourceLayoutElementDescription>& out_elements) const
  {
    // TODO
  }

  void spShaderProgramMTL::Use()
  {
    // TODO
  }

#pragma endregion

#pragma region spShaderMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spShaderMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    if (m_Description.m_bOwnBuffer)
      EZ_DEFAULT_DELETE_ARRAY(m_Description.m_Buffer);

    SP_RHI_MTL_RELEASE(m_pMTLShaderFunction);
    SP_RHI_MTL_RELEASE(m_pMTLShaderLibrary);

    m_bIsResourceCreated = false;
  }

  bool spShaderMTL::IsReleased() const
  {
    return m_pMTLShaderFunction == nullptr;
  }

  void spShaderMTL::SetDebugName(ezStringView sDebugName)
  {
    spShader::SetDebugName(sDebugName);

    if (IsReleased() || sDebugName.IsEmpty())
      return;

    const ezStringBuilder sName(sDebugName);

    NS::String* pString = NS::String::string(sName.GetData(), NS::UTF8StringEncoding);
    m_pMTLShaderLibrary->setLabel(pString);
    SP_RHI_MTL_RELEASE(pString);
  }

  void spShaderMTL::CreateResource()
  {
    NS::Error* pError = nullptr;
    spScopedMTLResource pEntryPoint(NS::String::string(m_Description.m_sEntryPoint.GetData(), NS::UTF8StringEncoding));

    // If the description contains a compiled shader binary, use it directly.
    if (m_Description.m_Buffer.GetCount() > 4 && m_Description.m_Buffer[0] == 0x4d && m_Description.m_Buffer[1] == 0x54 && m_Description.m_Buffer[2] == 0x4c && m_Description.m_Buffer[3] == 0x42)
    {
      const auto& queue = dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0);
      const auto& data = dispatch_data_create(m_Description.m_Buffer.GetPtr(), m_Description.m_Buffer.GetCount(), queue, nullptr);

      m_pMTLShaderLibrary = m_pMTLDevice->newLibrary(data, &pError);
      dispatch_release(data);
    }
    else
    {
      // Otherwise, compile the shader.
      const ezStringView sView(reinterpret_cast<const char*>(m_Description.m_Buffer.GetPtr()), m_Description.m_Buffer.GetCount());
      ezStringBuilder sTemp;

      spScopedMTLResource pShaderCode(NS::String::string(sView.GetData(sTemp), NS::UTF8StringEncoding));
      spScopedMTLResource pCompileOptions(MTL::CompileOptions::alloc()->init());

      m_pMTLShaderLibrary = m_pMTLDevice->newLibrary(*pShaderCode, *pCompileOptions, &pError);
    }

    if (pError != nullptr)
    {
      ezStringBuilder sError;
      sError.SetFormat("Failed to create shader library: {0}\n{1}", pError->localizedFailureReason()->utf8String(), pError->localizedDescription()->utf8String());

      EZ_LOG_BLOCK("Shader Compilation Error Message");
      ezLog::Dev("{0}", sError.GetData());

      SP_RHI_MTL_RELEASE(pError);
      return;
    }

    m_pMTLShaderFunction = m_pMTLShaderLibrary->newFunction(*pEntryPoint);

    if (m_pMTLShaderFunction == nullptr)
    {
      EZ_LOG_BLOCK("Shader Compilation Error Message");
      ezLog::Dev("Failed to create shader function: {0}", m_Description.m_sEntryPoint.GetData());
      return;
    }

    m_bHasFunctionConstants = m_pMTLShaderFunction->functionConstantsDictionary()->count() > 0;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    SetDebugName(m_sDebugName);
#endif

    m_bIsResourceCreated = true;
  }

  spShaderMTL::spShaderMTL(spDeviceMTL* pDevice, const spShaderDescription& description)
    : spShader(description)
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();
  }

  spShaderMTL::~spShaderMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Shader);
