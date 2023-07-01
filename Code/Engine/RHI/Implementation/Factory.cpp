#include <RHI/RHIPCH.h>

#include <RHI/Factory.h>

struct FactoryInfo
{
  spRHIImplementationFactory::Factory m_Func;
  ezString m_sShaderModel;
  ezString m_sShaderCompiler;
};

static ezHashTable<ezStringView, FactoryInfo> s_Factories;

FactoryInfo* GetFactoryInfo(ezStringView szImplementationName)
{
  auto pFactory = s_Factories.GetValue(szImplementationName);

  if (pFactory == nullptr)
  {
    ezStringBuilder sPluginName = "ezRHI";
    sPluginName.Append(szImplementationName);

    EZ_VERIFY(ezPlugin::LoadPlugin(sPluginName).Succeeded(), "Renderer plugin '{}' not found", sPluginName);

    pFactory = s_Factories.GetValue(szImplementationName);
    EZ_ASSERT_DEV(pFactory != nullptr, "Renderer '{}' is not registered", szImplementationName);
  }

  return pFactory;
}

ezInternal::NewInstance<RHI::spDevice> spRHIImplementationFactory::CreateDevice(ezStringView szImplementationName, ezAllocatorBase* pAllocator, const RHI::spDeviceDescription& description)
{
  if (auto pFuncInfo = GetFactoryInfo(szImplementationName))
  {
    return pFuncInfo->m_Func(pAllocator, description);
  }

  return {nullptr, pAllocator};
}

void spRHIImplementationFactory::RegisterImplementation(ezStringView szName, const Factory& func, const spRHIImplementationDescription& description)
{
  FactoryInfo info;
  info.m_Func = func;
  info.m_sShaderModel = description.m_sShaderModel;
  info.m_sShaderCompiler = description.m_sShaderCompiler;

  EZ_VERIFY(s_Factories.Insert(szName, info) == false, "Implementation already registered.");
}

void spRHIImplementationFactory::UnregisterImplementation(ezStringView szName)
{
  EZ_VERIFY(s_Factories.Remove(szName), "Implementation not registered.");
}

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Factory);