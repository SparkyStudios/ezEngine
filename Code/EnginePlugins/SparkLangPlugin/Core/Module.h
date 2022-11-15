#pragma once

#include <SparkLangPlugin/SparkLangPluginDLL.h>

#include <Foundation/Configuration/Singleton.h>

#include <sqmodules/sqmodules.h>
#include <sqrat.h>

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangModule
{
  EZ_DECLARE_SINGLETON(ezSparkLangModule);

public:
  ezSparkLangModule();

  void Register(SqModules* modules);

  SQRESULT ezLog(Sqrat::Table& module);
  SQRESULT ezClock(Sqrat::Table& module);
  SQRESULT ezComponent(Sqrat::Table& module);
};
