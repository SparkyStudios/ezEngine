#pragma once

#include <cstdarg>

#include <SparkLangPlugin/SparkLangPluginDLL.h>

#include <SparkLangPlugin/Core/Module.h>

#include <Core/World/World.h>
#include <Foundation/Strings/StringView.h>
#include <Foundation/Types/Types.h>

#include <squirrel.h>

namespace ezSparkLangInternal
{
  template <bool is_error>
  static void ezspark_print([[maybe_unused]] HSQUIRRELVM vm, const SQChar* format, ...)
  {
    if (format && format[0] != '\0')
    {
      constexpr size_t bufferLen = 1024;
      char buffer[bufferLen] = "";

      va_list args;
      va_start(args, format);
      vsnprintf(buffer, bufferLen, format, args);
      va_end(args);

      buffer[bufferLen - 1] = '\0';

      if constexpr (is_error)
      {
        ezLog::Error(buffer);
      }
      else
      {
        ezLog::Dev(buffer);
      }
    }
  }
} // namespace ezSparkLangInternal

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptContext
{
public:
  struct PropertyBinding
  {
    ezAbstractMemberProperty* m_pProperty = nullptr;
  };

  struct FunctionBinding
  {
    ezAbstractFunctionProperty* m_pFunction = nullptr;
  };

  static ezSparkLangScriptContext* FromVM(HSQUIRRELVM vm);

  static const PropertyBinding* FindPropertyBinding(ezUInt32 uiHash);
  static const FunctionBinding* FindFunctionBinding(ezUInt32 uiHash);

  static void GenerateComponentScripts();
  static void GenerateMessageScripts();

  ezSparkLangScriptContext();
  ~ezSparkLangScriptContext();

  ezResult Initialize(ezWorld* pWorld, HSQUIRRELVM vm = nullptr);
  ezResult Run(ezStringView svScript, Sqrat::Object* context = nullptr) const;
  void CollectGarbage() const;

  EZ_ALWAYS_INLINE ezWorld* GetWorld() const
  {
    return m_pWorld;
  }

  EZ_ALWAYS_INLINE HSQUIRRELVM GetVM() const
  {
    return m_vm;
  }

private:
  friend class ezSparkLangScriptComponentManager;

  static void SetupComponentFunctionBindings();
  static void SetupComponentPropertyBindings();

  static ezHashTable<ezUInt32, PropertyBinding> s_BoundProperties;
  static ezHashTable<ezUInt32, FunctionBinding> s_BoundFunctions;

  ezWorld* m_pWorld;

  HSQUIRRELVM m_vm;
  bool m_bIsCustomVm;

  SqModules m_ModulesManager;
  ezSparkLangModule m_ezModule;
};
