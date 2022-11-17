#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Allocator.h>
#include <SparkLangPlugin/Core/ScriptContext.h>

#include <Foundation/Configuration/CVar.h>

#include <sqrat.h>
#include <squirrel.h>

static constexpr char s_szScriptContextRegistrySlotName[] = "_spark_script_context";
static constexpr char s_szComponentsRootTableSlotName[] = "_spark_script_components";

ezCVarInt cvar_SparkLangVMInitialStackSize = ezCVarInt("SparkLang.InitialStackSize", 1024, ezCVarFlags::Save, "The initial size of the VM stack.");

static void DebugHook(HSQUIRRELVM vm, SQInteger type, const SQChar* name, SQInteger line, const SQChar* funcName)
{
  ezLog::Info("type: {} name: {} line: {} func: {}", type, name, line, funcName);
}

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

ezSparkLangScriptContext::ezSparkLangScriptContext()
  : m_pWorld(nullptr)
  , m_vm(nullptr)
  , m_bIsCustomVm(false)
  , m_ModulesManager(nullptr)
{
}

ezSparkLangScriptContext::~ezSparkLangScriptContext()
{
  if (m_bIsCustomVm && m_vm != nullptr)
  {
    sq_close(m_vm);
  }
}

ezResult ezSparkLangScriptContext::Initialize(ezWorld* pWorld, HSQUIRRELVM vm)
{
  m_pWorld = pWorld;
  m_vm = vm;
  m_bIsCustomVm = vm != nullptr;

  if (!m_bIsCustomVm)
  {
    m_vm = sq_open(cvar_SparkLangVMInitialStackSize); // creates a VM with initial stack size 1024

    sq_setprintfunc(m_vm, &ezSparkLangInternal::ezspark_print<false>, &ezSparkLangInternal::ezspark_print<true>);
    sq_setnativedebughook(m_vm, DebugHook);
  }

  m_ModulesManager = SqModules(m_vm);

  if (!m_bIsCustomVm)
  {
    m_ModulesManager.setupMainModule();
    m_ModulesManager.registerBaseLibs();
    m_ModulesManager.registerDateTimeLib();
  }

  // Register the ez module
  m_ezModule.Register(m_ModulesManager);

  Sqrat::RootTable rootTable(m_vm);

  sq_pushregistrytable(m_vm);
  sq_pushstring(m_vm, s_szScriptContextRegistrySlotName, sizeof(s_szScriptContextRegistrySlotName) - 1);
  sq_pushuserpointer(m_vm, this);
  sq_newslot(m_vm, -3, SQFalse);
  sq_poptop(m_vm); // pop the registry table

  if (!m_bIsCustomVm)
  {
    rootTable.SetValue(s_szComponentsRootTableSlotName, Sqrat::Table(m_vm));
  }

  return EZ_SUCCESS;
}

ezResult ezSparkLangScriptContext::Run(ezStringView svScript, Sqrat::Object* context) const
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

void ezSparkLangScriptContext::CollectGarbage() const
{
  if (m_vm == nullptr)
    return;

  sq_collectgarbage(m_vm);
}
