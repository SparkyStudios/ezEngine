#pragma once

#include <SparkLangPlugin/SparkLangPluginDLL.h>

#include <Foundation/Configuration/Singleton.h>

#include <sqmodules/sqmodules.h>
#include <sqrat.h>

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangModule
{
public:
  ezSparkLangModule();

  bool Register(SqModules& modules);

  SQRESULT ezLog(Sqrat::Table& module);
  SQRESULT ezClock(Sqrat::Table& module);
  SQRESULT ezComponent(Sqrat::Table& module);
  SQRESULT ezGameObject(Sqrat::Table& module);
  SQRESULT ezMath(Sqrat::Table& module);
  SQRESULT ezRandom(Sqrat::Table& module);
  SQRESULT ezWorld(Sqrat::Table& module);
};
