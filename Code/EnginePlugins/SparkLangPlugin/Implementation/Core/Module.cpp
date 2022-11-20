#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Module.h>

#include <Core/World/World.h>

ezSparkLangModule::ezSparkLangModule()
{
}

bool ezSparkLangModule::Register(SqModules& modules)
{
  HSQOBJECT hModule;
  sq_newtable(modules.getVM());
  sq_getstackobj(modules.getVM(), -1, &hModule);

  {
    Sqrat::Table module(hModule, modules.getVM());

    if (SQ_FAILED(ezClock(module)))
      return false;

    if (SQ_FAILED(ezComponent(module)))
      return false;

    if (SQ_FAILED(ezGameObject(module)))
      return false;

    if (SQ_FAILED(ezLog(module)))
      return false;

    if (SQ_FAILED(ezMath(module)))
      return false;

    if (SQ_FAILED(ezWorld(module)))
      return false;
  }

  const bool bDone = modules.addNativeModule("ez", SqModules::SqObjPtr(modules.getVM(), hModule));

  sq_poptop(modules.getVM());
  return bDone;
}
