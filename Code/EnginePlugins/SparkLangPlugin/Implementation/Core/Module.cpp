#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Module.h>

#include <Core/World/World.h>

// clang-format off
EZ_IMPLEMENT_SINGLETON(ezSparkLangModule);
// clang-format on

static SQRESULT RegisterInternal(HSQUIRRELVM vm)
{
  if (auto* pModule = ezSparkLangModule::GetSingleton(); pModule != nullptr)
  {
    HSQOBJECT moduleObj;
    sq_getstackobj(vm, -1, &moduleObj);
    Sqrat::Table module(moduleObj, vm);

    if (SQ_FAILED(pModule->ezClock(module)))
      return SQ_ERROR;

    if (SQ_FAILED(pModule->ezComponent(module)))
      return SQ_ERROR;

    if (SQ_FAILED(pModule->ezLog(module)))
      return SQ_ERROR;

    return SQ_OK;
  }

  return SQ_ERROR;
}

ezSparkLangModule::ezSparkLangModule()
  : m_SingletonRegistrar(this)
{
}

void ezSparkLangModule::Register(SqModules* modules)
{
  modules->registerNativeModule("ez", RegisterInternal);
}
