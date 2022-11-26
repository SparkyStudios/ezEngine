#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Allocator.h>
#include <SparkLangPlugin/Core/ScriptContext.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

#include <sqrat.h>
#include <squirrel.h>

static constexpr char s_szScriptContextRegistrySlotName[] = "_spark_script_context";
static constexpr char s_szComponentsRootTableSlotName[] = "_spark_script_components";

ezCVarInt cvar_SparkLangVMInitialStackSize = ezCVarInt("SparkLang.InitialStackSize", 1024, ezCVarFlags::Save, "The initial size of the VM stack.");

static void DebugHook(HSQUIRRELVM vm, SQInteger type, const SQChar* name, SQInteger line, const SQChar* funcName)
{
  ezLog::Info("type: {} name: {} line: {} func: {}", type, name, line, funcName);
}

static ezUInt32 ComputePropertyBindingHash(const ezRTTI* pType, const ezAbstractProperty* pProperty)
{
  ezStringBuilder sFuncName;

  sFuncName.Set(pType->GetTypeName(), "::", pProperty->GetPropertyName());

  return ezHashingUtils::StringHashTo32(ezHashingUtils::StringHash(sFuncName.GetData()));
}

static void CreateComponentTypeList(ezSet<const ezRTTI*>& found, ezDynamicArray<const ezRTTI*>& sorted, const ezRTTI* pRtti)
{
  if (found.Contains(pRtti))
    return;

  if (!pRtti->IsDerivedFrom<ezComponent>())
    return;

  if (pRtti == ezGetStaticRTTI<ezComponent>() || pRtti == ezGetStaticRTTI<ezSparkLangScriptComponent>())
    return;

  found.Insert(pRtti);
  CreateComponentTypeList(found, sorted, pRtti->GetParentType());

  sorted.PushBack(pRtti);
}

static void CreateMessageTypeList(ezSet<const ezRTTI*>& found, ezDynamicArray<const ezRTTI*>& sorted, const ezRTTI* pRtti)
{
  if (found.Contains(pRtti))
    return;

  if (!pRtti->IsDerivedFrom<ezMessage>())
    return;

  if (pRtti == ezGetStaticRTTI<ezMessage>() || pRtti == ezGetStaticRTTI<ezEventMessage>() || pRtti == ezGetStaticRTTI<ezSparkLangScriptMessageProxy>() || pRtti == ezGetStaticRTTI<ezSparkLangScriptEventMessageProxy>())
    return;

  found.Insert(pRtti);
  CreateMessageTypeList(found, sorted, pRtti->GetParentType());

  sorted.PushBack(pRtti);
}

static void GenerateConstructorString(ezStringBuilder& out_String, const ezVariant& value)
{
  out_String.Set("null");

  switch (value.GetType())
  {
    case ezVariant::Type::Invalid:
      break;

    case ezVariant::Type::Bool:
    case ezVariant::Type::Int8:
    case ezVariant::Type::UInt8:
    case ezVariant::Type::Int16:
    case ezVariant::Type::UInt16:
    case ezVariant::Type::Int32:
    case ezVariant::Type::UInt32:
    case ezVariant::Type::Int64:
    case ezVariant::Type::UInt64:
    case ezVariant::Type::Float:
    case ezVariant::Type::Double:
    {
      out_String = value.ConvertTo<ezString>();
      break;
    }

    case ezVariant::Type::String:
    case ezVariant::Type::StringView:
    {
      out_String.Format("\"{}\"", value.ConvertTo<ezString>());
      break;
    }

    case ezVariant::Type::Color:
    case ezVariant::Type::ColorGamma:
    {
      const auto& c = value.ConvertTo<ezColor>();
      out_String.Format("ezColor({}, {}, {}, {})", c.r, c.g, c.b, c.a);
      break;
    }

    case ezVariant::Type::Vector2:
    {
      const auto& v = value.Get<ezVec2>();
      out_String.Format("ezVec2({}, {})", v.x, v.y);
      break;
    }

    case ezVariant::Type::Vector3:
    {
      const auto& v = value.Get<ezVec3>();
      out_String.Format("ezVec3({}, {}, {})", v.x, v.y, v.z);
      break;
    }

    case ezVariant::Type::Vector4:
    {
      const auto& v = value.Get<ezVec4>();
      out_String.Format("ezVec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      break;
    }

    case ezVariant::Type::Vector2I:
    {
      const auto& v = value.Get<ezVec2I32>();
      out_String.Format("ezVec2({}, {})", v.x, v.y);
      break;
    }

    case ezVariant::Type::Vector3I:
    {
      const auto& v = value.Get<ezVec3I32>();
      out_String.Format("ezVec3({}, {}, {})", v.x, v.y, v.z);
      break;
    }

    case ezVariant::Type::Vector4I:
    {
      const auto& v = value.Get<ezVec4I32>();
      out_String.Format("ezVec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      break;
    }

    case ezVariant::Type::Vector2U:
    {
      const auto& v = value.Get<ezVec2U32>();
      out_String.Format("ezVec2({}, {})", v.x, v.y);
      break;
    }

    case ezVariant::Type::Vector3U:
    {
      const auto& v = value.Get<ezVec3U32>();
      out_String.Format("ezVec3({}, {}, {})", v.x, v.y, v.z);
      break;
    }

    case ezVariant::Type::Vector4U:
    {
      const auto& v = value.Get<ezVec4U32>();
      out_String.Format("ezVec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      break;
    }

    case ezVariant::Type::Quaternion:
    {
      const auto& q = value.Get<ezQuat>();
      out_String.Format("ezQuat({}, {}, {}, {})", q.v.x, q.v.y, q.v.z, q.w);
      break;
    }

    case ezVariant::Type::Matrix3:
    {
      out_String = "ezMat3()";
      break;
    }

    case ezVariant::Type::Matrix4:
    {
      out_String = "ezMat4()";
      break;
    }

    case ezVariant::Type::Transform:
    {
      out_String = "ezTransform()";
      break;
    }

    case ezVariant::Type::Time:
      out_String.Format("ezTime.Seconds({0})", value.Get<ezTime>().GetSeconds());
      break;

    case ezVariant::Type::Angle:
      out_String.Format("ezAngle.Radian({0})", value.Get<ezAngle>().GetRadian());
      break;

    case ezVariant::Type::Uuid:
    case ezVariant::Type::DataBuffer:
    case ezVariant::Type::VariantArray:
    case ezVariant::Type::VariantDictionary:
    case ezVariant::Type::TypedPointer:
    case ezVariant::Type::TypedObject:
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      break;
  }
}

static void GenerateExposedFunctionsCode(ezStringBuilder& out_Code, const ezRTTI* pRtti)
{
  ezStringBuilder sFunc;

  for (ezAbstractFunctionProperty* pFunc : pRtti->GetFunctions())
  {
    // TODO: static members ?
    if (pFunc->GetFunctionType() != ezFunctionType::Member)
      continue;

    const auto* pAttr = pFunc->GetAttributeByType<ezScriptableFunctionAttribute>();

    if (pAttr == nullptr)
      goto ignore;

    sFunc.Set("  function ", pFunc->GetPropertyName(), "(");

    for (ezUInt32 i = 0; i < pFunc->GetArgumentCount(); ++i)
    {
      sFunc.Append(i > 0 ? ", " : "", pAttr->GetArgumentName(i));
    }

    sFunc.Append(")");

    {
      const bool bHasReturnValue = pFunc->GetReturnType() != nullptr;

      // function body
      {
        ezUInt32 uiFuncHash = ComputePropertyBindingHash(pRtti, pFunc);

        if (bHasReturnValue)
          sFunc.AppendFormat(" { return ez.Component.CallFunc(GetHandle(), {0}", uiFuncHash);
        else
          sFunc.AppendFormat(" { ez.Component.CallFunc(GetHandle(), {0}", uiFuncHash);

        for (ezUInt32 arg = 0; arg < pFunc->GetArgumentCount(); ++arg)
        {
          sFunc.Append(", ", pAttr->GetArgumentName(arg));
        }

        sFunc.Append(") }\n");
      }
    }

    out_Code.Append(sFunc.GetView());

  ignore:
    continue;
  }
}

static void GeneratePropertiesCode(ezStringBuilder& out_Code, const ezRTTI* pRtti)
{
  ezStringBuilder sGetter, sSetter;

  sGetter.Set("  function _get(prop) {\n");
  sSetter.Set("  function _set(prop, value) {\n");

  ezUInt32 i = 0;

  for (ezAbstractProperty* pProp : pRtti->GetProperties())
  {
    if (pProp->GetCategory() != ezPropertyCategory::Member)
      continue;

    const auto* pMember = static_cast<ezAbstractMemberProperty*>(pProp);
    const ezUInt32 uiHash = ComputePropertyBindingHash(pRtti, pMember);

    sGetter.AppendFormat("    if (prop == \"{0}\") { return ez.Component.GetProp(GetHandle(), {1}) }\n", pMember->GetPropertyName(), uiHash);

    if (pProp->GetFlags().IsSet(ezPropertyFlags::ReadOnly))
      continue;

    sSetter.AppendFormat("    {2} (prop == \"{0}\") { ez.Component.SetProp(GetHandle(), {1}, value) }\n", pMember->GetPropertyName(), uiHash, i == 0 ? "if" : "else if");

    i++;
  }

  sGetter.Append("  }\n");
  sSetter.Append("  }\n");

  if (i > 0)
  {
    out_Code.Append(sGetter.GetView());
    out_Code.Append(sSetter.GetView());
  }
}

static void GenerateMessagePropertiesCode(ezStringBuilder& out_Code, const ezRTTI* pRtti)
{
  ezStringBuilder sProp;
  ezStringBuilder sDefault = "null";

  for (ezAbstractProperty* pProp : pRtti->GetProperties())
  {
    if (pProp->GetCategory() != ezPropertyCategory::Member)
      continue;

    const auto* pMember = static_cast<ezAbstractMemberProperty*>(pProp);

    const ezVariant defaultValue = ezReflectionUtils::GetDefaultValue(pMember);
    GenerateConstructorString(sDefault, defaultValue);

    sProp.Format("  {0} = {1}\n", pMember->GetPropertyName(), sDefault);

    out_Code.Append(sProp.GetView());
  }
}

static void GenerateComponentCode(const ezRTTI* pRtti)
{
  ezStringBuilder sPath;

  {
    ezStringBuilder sPluginName(pRtti->GetPluginName());
    sPluginName.TrimWordStart("ez");
    sPluginName.TrimWordEnd("Plugin");

    ezStringBuilder sFileName(pRtti->GetTypeName());
    sFileName.TrimWordStart("ez");

    sPath.AppendFormat(">project/SparkLang/{0}", sPluginName);

    if (ezFileSystem::CreateDirectoryStructure(sPath).Failed())
      return;

    sPath.ReplaceFirst(">"_ezsv, ":"_ezsv);
    sPath.AppendFormat("/{0}.spark", sFileName);
  }

  ezFileWriter file;
  if (file.Open(sPath).Failed())
    return;

  ezStringBuilder sComponentType(pRtti->GetTypeName()), sParentType(pRtti->GetParentType()->GetTypeName()), sCode;

  sCode.AppendFormat("::{0} <- class extends {1} {\n", sComponentType, sParentType);
  sCode.AppendFormat("  static function GetTypeName() { return \"{0}\"; }\n", sComponentType);
  sCode.AppendFormat("  static function GetTypeNameHash() { return {0}; }\n", ezHashingUtils::StringHashTo32(pRtti->GetTypeNameHash()));
  GenerateExposedFunctionsCode(sCode, pRtti);
  GeneratePropertiesCode(sCode, pRtti);
  sCode.Append("}\n\n");

  file.WriteBytes(sCode, sCode.GetElementCount()).IgnoreResult();
}

static void GenerateMessageCode(const ezRTTI* pRtti)
{
  ezStringBuilder sPath;

  {
    ezStringBuilder sPluginName(pRtti->GetPluginName());
    sPluginName.TrimWordStart("ez");
    sPluginName.TrimWordEnd("Plugin");

    ezStringBuilder sFileName(pRtti->GetTypeName());
    sFileName.TrimWordStart("ez");

    sPath.AppendFormat(">project/SparkLang/{0}", sPluginName);

    if (ezFileSystem::CreateDirectoryStructure(sPath).Failed())
      return;

    sPath.ReplaceFirst(">"_ezsv, ":"_ezsv);
    sPath.AppendFormat("/{0}.spark", sFileName);
  }

  ezFileWriter file;
  if (file.Open(sPath).Failed())
    return;

  ezStringBuilder sMessageType(pRtti->GetTypeName()), sParentType(pRtti->GetParentType()->GetTypeName()), sCode;

  sCode.AppendFormat("::{0} <- class extends {1} {\n", sMessageType, sParentType);
  sCode.AppendFormat("  static function GetTypeName() { return \"{0}\"; }\n", sMessageType);
  sCode.AppendFormat("  static function GetTypeNameHash() { return {0}; }\n", ezHashingUtils::StringHashTo32(pRtti->GetTypeNameHash()));
  GenerateMessagePropertiesCode(sCode, pRtti);
  sCode.Append("}\n\n");

  file.WriteBytes(sCode, sCode.GetElementCount()).IgnoreResult();
}

ezHashTable<ezUInt32, ezSparkLangScriptContext::FunctionBinding> ezSparkLangScriptContext::s_BoundFunctions;
ezHashTable<ezUInt32, ezSparkLangScriptContext::PropertyBinding> ezSparkLangScriptContext::s_BoundProperties;

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

const ezSparkLangScriptContext::PropertyBinding* ezSparkLangScriptContext::FindPropertyBinding(ezUInt32 uiHash)
{
  const PropertyBinding* pBinding;
  s_BoundProperties.TryGetValue(uiHash, pBinding);
  return pBinding;
}

const ezSparkLangScriptContext::FunctionBinding* ezSparkLangScriptContext::FindFunctionBinding(ezUInt32 uiHash)
{
  const FunctionBinding* pBinding;
  s_BoundFunctions.TryGetValue(uiHash, pBinding);
  return pBinding;
}

void ezSparkLangScriptContext::GenerateComponentScripts()
{
  ezSet<const ezRTTI*> found;
  ezDynamicArray<const ezRTTI*> sorted;
  sorted.Reserve(100);

  ezHybridArray<const ezRTTI*, 64> alphabetical;
  for (const auto& pRtti : ezRTTI::GetAllTypesDerivedFrom(ezGetStaticRTTI<ezComponent>(), alphabetical, true))
  {
    CreateComponentTypeList(found, sorted, pRtti);
  }

  for (const ezRTTI* pRtti : sorted)
  {
    GenerateComponentCode(pRtti);
  }
}

void ezSparkLangScriptContext::GenerateMessageScripts()
{
  ezSet<const ezRTTI*> found;
  ezDynamicArray<const ezRTTI*> sorted;
  sorted.Reserve(100);

  ezHybridArray<const ezRTTI*, 64> alphabetical;
  for (const auto& pRtti : ezRTTI::GetAllTypesDerivedFrom(ezGetStaticRTTI<ezMessage>(), alphabetical, true))
  {
    if (pRtti == ezGetStaticRTTI<ezMessage>() || pRtti == ezGetStaticRTTI<ezEventMessage>())
      continue;

    CreateMessageTypeList(found, sorted, pRtti);
  }

  for (auto&& pRtti : sorted)
  {
    GenerateMessageCode(pRtti);
  }
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

  ezFileReader file;
  if (file.Open(":project/script.ezSparkLangScript").Succeeded())
  {
    scriptObj.Deserialize(
      err, [](SQUserPointer up, SQUserPointer data, SQInteger size) -> SQInteger
      { auto* file = static_cast<ezFileReader*>(up);
        return file->ReadBytes(data, size); },
      &file);
  }
  else if (scriptObj.CompileString(sScript.GetData(), err, "script.spark"))
  {
    ezFileWriter file;
    EZ_SUCCEED_OR_RETURN(file.Open(":project/script.ezSparkLangScript"));

    scriptObj.Serialize(
      err, [](SQUserPointer up, SQUserPointer data, SQInteger size) -> SQInteger
      { auto* file = static_cast<ezFileWriter*>(up);
        return  file->WriteBytes(data, size).Succeeded() ? size : 0; },
      &file);
  }

  if (scriptObj.Run(err, context))
  {
    return EZ_SUCCESS;
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

void ezSparkLangScriptContext::SetupComponentFunctionBindings()
{
  if (!s_BoundFunctions.IsEmpty())
    return;

  for (const ezRTTI* pRtti = ezRTTI::GetFirstInstance(); pRtti != nullptr; pRtti = pRtti->GetNextInstance())
  {
    if (!pRtti->IsDerivedFrom<ezComponent>())
      continue;

    for (ezAbstractFunctionProperty* pFunc : pRtti->GetFunctions())
    {
      // TODO: static members ?
      if (pFunc->GetFunctionType() != ezFunctionType::Member)
        continue;

      const ezUInt32 uiHash = ComputePropertyBindingHash(pRtti, pFunc);
      EZ_ASSERT_DEV(!s_BoundFunctions.Contains(uiHash), "Hash collision for bound function name!");

      s_BoundFunctions[uiHash].m_pFunction = pFunc;
    }
  }
}

void ezSparkLangScriptContext::SetupComponentPropertyBindings()
{
  if (!s_BoundProperties.IsEmpty())
    return;

  for (const ezRTTI* pRtti = ezRTTI::GetFirstInstance(); pRtti != nullptr; pRtti = pRtti->GetNextInstance())
  {
    if (!pRtti->IsDerivedFrom<ezComponent>())
      continue;

    for (ezAbstractProperty* pProperty : pRtti->GetProperties())
    {
      if (pProperty->GetCategory() != ezPropertyCategory::Member)
        continue;

      const ezUInt32 uiHash = ComputePropertyBindingHash(pRtti, pProperty);
      EZ_ASSERT_DEV(!s_BoundProperties.Contains(uiHash), "Hash collision for bound function name!");

      s_BoundProperties[uiHash].m_pProperty = static_cast<ezAbstractMemberProperty*>(pProperty);
    }
  }
}
