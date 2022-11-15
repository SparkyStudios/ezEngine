#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Allocator.h>
#include <SparkLangPlugin/Core/ScriptContext.h>

#include <Foundation/Configuration/CVar.h>

#include <sqrat.h>
#include <squirrel.h>

static constexpr char s_szScriptContextRegistrySlotName[] = "_spark_script_context";

ezCVarInt cvar_SparkLangVMInitialStackSize = ezCVarInt("SparkLang.InitialStackSize", 1024, ezCVarFlags::Save, "The initial size of the VM stack.");

ezSparkLangScriptContext* ezSparkLangScriptContext::FromVM(HSQUIRRELVM vm)
{
  const Sqrat::RegistryTable registry(vm);
  SQUserPointer pUserPointer = sq_objtouserpointer(&registry.RawGetSlot(s_szScriptContextRegistrySlotName).GetObject());

  if (pUserPointer == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return nullptr;
  }

  return static_cast<ezSparkLangScriptContext*>(pUserPointer);
}

ezSparkLangScriptContext::ezSparkLangScriptContext(ezWorld* pWorld, HSQUIRRELVM vm)
  : m_pWorld(pWorld)
  , m_vm(vm)
  , m_bIsCustomVm(vm != nullptr)
  , m_ezModule(ezSparkLangModule::GetSingleton())
{
  if (!m_bIsCustomVm)
  {
    m_vm = sq_open(cvar_SparkLangVMInitialStackSize); // creates a VM with initial stack size 1024

    sq_setprintfunc(m_vm, &ezSparkLangInternal::ezspark_print<false>, &ezSparkLangInternal::ezspark_print<true>);
  }

  m_pModulesManager = EZ_DEFAULT_NEW(SqModules, m_vm);

  if (!m_bIsCustomVm)
  {
    m_pModulesManager->setupMainModule();
    m_pModulesManager->registerBaseLibs();
    m_pModulesManager->registerDateTimeLib();
  }

  if (m_ezModule == nullptr)
    m_ezModule = new ezSparkLangModule();

  // Register the ez module
  m_ezModule->Register(m_pModulesManager);

  Sqrat::RootTable rootTable(m_vm);

  sq_pushregistrytable(m_vm);
  sq_pushstring(m_vm, s_szScriptContextRegistrySlotName, sizeof(s_szScriptContextRegistrySlotName) - 1);
  sq_pushuserpointer(m_vm, this);
  sq_newslot(m_vm, -3, SQFalse);
  sq_pop(m_vm, 1); // pop the registry table

  if (!m_bIsCustomVm)
  {
    // rootTable.SquirrelFunc(_SC("_get"), &RootTable__get);
  }
}

ezSparkLangScriptContext::~ezSparkLangScriptContext()
{
  EZ_DEFAULT_DELETE(m_pModulesManager);

  if (m_bIsCustomVm)
  {
    sq_close(m_vm);
  }
}

ezResult ezSparkLangScriptContext::Run(ezStringView svScript, Sqrat::Object* context)
{
  Sqrat::Script scriptObj(m_vm);

  const ezStringBuilder sScript(svScript);
  Sqrat::string err{};

  if (scriptObj.CompileString(sScript.GetData(), err, "script.spark"))
  {
    if (scriptObj.Run(err, context))
    {
      return EZ_SUCCESS;
    }
  }

  ezLog::Error(err.c_str());
  return EZ_FAILURE;
}
