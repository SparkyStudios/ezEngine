#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>

ezSparkLangScriptComponentManager::ezSparkLangScriptComponentManager(ezWorld* pWorld)
  : SUPER(pWorld)
{
  m_ScriptContext.Initialize(GetWorld()).IgnoreResult();
}

ezSparkLangScriptComponentManager::~ezSparkLangScriptComponentManager() = default;

void ezSparkLangScriptComponentManager::Initialize()
{
  SUPER::Initialize();

  ezSparkLangScriptContext::SetupComponentFunctionBindings();
  ezSparkLangScriptContext::SetupComponentPropertyBindings();

  ezSparkLangScriptContext::GenerateComponentScripts();
  ezSparkLangScriptContext::GenerateMessageScripts();

  auto desc = EZ_CREATE_MODULE_UPDATE_FUNCTION_DESC(ezSparkLangScriptComponentManager::Update, this);
  desc.m_bOnlyUpdateWhenSimulating = true;
  desc.m_Phase = UpdateFunctionDesc::Phase::PreAsync;

  RegisterUpdateFunction(desc);
}

void ezSparkLangScriptComponentManager::Deinitialize()
{
  SUPER::Deinitialize();
}

void ezSparkLangScriptComponentManager::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();
}

void ezSparkLangScriptComponentManager::Update(const ezWorldModule::UpdateContext& context)
{
  EZ_PROFILE_SCOPE("SparkLang Update");

  for (auto it = this->m_ComponentStorage.GetIterator(context.m_uiFirstComponentIndex, context.m_uiComponentCount); it.IsValid(); ++it)
  {
    if (it->IsActiveAndSimulating())
    {
      it->Update();
    }
  }

  m_ScriptContext.CollectGarbage();
}
