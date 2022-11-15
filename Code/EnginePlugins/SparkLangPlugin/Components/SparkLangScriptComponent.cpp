#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>

#include <sqimportparser.h>
#include <sqrat.h>
#include <squirrel.h>

void error_cb(void* /*user_pointer*/, const char* message, int line, int column)
{
  ezLog::Error("messsage: {} line: {} col: {}", message, line, column);
}

ezResult ReadEntireFile(const char* szFile, ezStringBuilder& sOut)
{
  sOut.Clear();

  ezFileReader File;
  if (File.Open(szFile) == EZ_FAILURE)
  {
    ezLog::Error("Could not open for reading: '{0}'", szFile);
    return EZ_FAILURE;
  }

  ezDynamicArray<ezUInt8> FileContent;

  ezUInt8 Temp[4024];
  ezUInt64 uiRead = File.ReadBytes(Temp, EZ_ARRAY_SIZE(Temp));

  while (uiRead > 0)
  {
    FileContent.PushBackRange(ezArrayPtr<ezUInt8>(Temp, (ezUInt32)uiRead));

    uiRead = File.ReadBytes(Temp, EZ_ARRAY_SIZE(Temp));
  }

  FileContent.PushBack(0);

  if (!ezUnicodeUtils::IsValidUtf8((const char*)&FileContent[0]))
  {
    ezLog::Error("The file \"{0}\" contains characters that are not valid Utf8. This often happens when you type special characters in "
                 "an editor that does not save the file in Utf8 encoding.",
      szFile);
    return EZ_FAILURE;
  }

  sOut = (const char*)&FileContent[0];

  return EZ_SUCCESS;
}

// clang-format off
EZ_IMPLEMENT_MESSAGE_TYPE(ezSparkLangScriptMessageProxy);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptMessageProxy, 1, ezRTTIDefaultAllocator<ezSparkLangScriptMessageProxy>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_IMPLEMENT_MESSAGE_TYPE(ezSparkLangScriptEventMessageProxy);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptEventMessageProxy, 1, ezRTTIDefaultAllocator<ezSparkLangScriptEventMessageProxy>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_COMPONENT_TYPE(ezSparkLangScriptComponent, 1, ezComponentMode::Static)
  {
  EZ_BEGIN_PROPERTIES
  {
    EZ_ARRAY_MEMBER_PROPERTY("Properties", m_Properties)->AddAttributes(new ezContainerAttribute(false, false, false))->AddFlags(ezPropertyFlags::PointerOwner),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezSparkLangScriptMessageProxy, OnMsgSparkLangScriptMessageProxy),
  }
  EZ_END_MESSAGEHANDLERS;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

static constexpr char Test[] = R""""(
import "SparkLang/Component.spark"

let class TestMessage extends ezMessage {
  entity = ""
  damage = 0

  constructor() {
    base.constructor()

    damage = ez.Component.GetUniqueID()
    entity = "npc"
  }

  function _typeof() {
    return "TestMessage"
  }
}

let class TestEvent extends ezEventMessage {
  hit = 0

  constructor() {
    base.constructor()

    hit = ez.Component.GetUniqueID()
  }

  function _typeof() {
    return "TestEvent"
  }
}

local once = false

let class TestComponent extends ezComponent
</ UpdateInterval = 10000 />
{
  </ Expose = true />
  Count = 0

  </ Expose = true />
  Name = "ezComponent Test"

  constructor() {
    base.constructor()
  }

  function Initialize() {
    ez.Log.Success("Initlaized")
  }

  function Deinitialize() {
    ez.Log.Success("Deinitlaized")
  }

  function OnActivated() {
    let ID = GetUniqueID()
    ez.Log.Success("Activated")
  }

  function OnDeactivated() {
    ez.Log.Success("Deactivated")
  }

  function OnSimulationStarted() {
    ez.Log.Success("SimulationStarted")
  }

  function Update() {
    Count++
    ez.Log.Success($"Update {Name} {Count} {IsActive()}")

    if (!once) {
      let msg = TestMessage()
      ez.Log.Info($"Sending Message from {GetUniqueID()}")
      SendMessage(msg)

      let event = TestEvent()
      ez.Log.Info($"Sending Event from {GetUniqueID}")
      BroadcastEvent(event)

      // once = true
    }
  }

  </ MessageHandler = "TestMessage" />
  function OnTestMessage(msg) {
    ez.Log.Info($"Got Message from {msg.damage} to {GetUniqueID()}")
    if (msg instanceof TestMessage) {
      ez.Log.Success($"Seriously got a TestMessage from C++ {msg.damage}")
    }
  }

  </ MessageHandler = "TestEvent" />
  function OnTestEvent(evt) {
    ez.Log.Info("Got a TestEvent from C++")
    if (evt instanceof TestEvent) {
      ez.Log.Success($"Successfully got the event from another script ({ez.Component.GetUniqueID()}), hit = {evt.hit}")
    }
  }
}

this.Component <- TestComponent()
)"""";

ezSparkLangScriptComponent::ezSparkLangScriptComponent()
  : ezEventMessageHandlerComponent()
{
}

ezSparkLangScriptComponent::~ezSparkLangScriptComponent() = default;

void ezSparkLangScriptComponent::BroadcastEventMessage(ezEventMessage& msg)
{
  const ezRTTI* pType = msg.GetDynamicRTTI();

  for (auto& sender : m_EventSenders)
  {
    if (sender.m_pMsgType == pType)
    {
      sender.m_Sender.SendEventMessage(msg, this, GetOwner()->GetParent());
      return;
    }
  }

  auto& sender = m_EventSenders.ExpandAndGetRef();
  sender.m_pMsgType = pType;
  sender.m_Sender.SendEventMessage(msg, this, GetOwner()->GetParent());
}

void ezSparkLangScriptComponent::SetUpdateInterval(float fIntervalMS)
{
  SetUpdateInterval(ezTime::Milliseconds(fIntervalMS));
}

void ezSparkLangScriptComponent::SetUpdateInterval(ezTime interval)
{
  m_UpdateInterval = interval;
}

ezTime ezSparkLangScriptComponent::GetUpdateInterval() const
{
  return m_UpdateInterval;
}

void ezSparkLangScriptComponent::Initialize()
{
  SUPER::Initialize();

  m_pScriptContext = EZ_DEFAULT_NEW(ezSparkLangScriptContext, GetWorld());

  Sqrat::RootTable root(m_pScriptContext->GetVM());
  root.SetValue<const ezComponentHandle&>(_SC("componentId"), GetHandle());
  root.SetValue<const ezGameObjectHandle&>(_SC("gameObjectId"), GetOwner()->GetHandle());

  m_ComponentScope = Sqrat::Table(m_pScriptContext->GetVM());
  root.SetValue(GetUniqueID(), m_ComponentScope);

  ezStringBuilder scriptCode;
  scriptCode.Append("let ez = require(\"ez\")\n\n");

  sqimportparser::ImportParser importParser(error_cb, nullptr);
  const char* txt = Test;
  int line = 1;
  int col = 1;
  std::vector<sqimportparser::ModuleImport> modules;
  std::vector<std::string> directives;
  std::vector<std::pair<const char*, const char*>> keepRanges; // .first - inclusive, .second - not inclusive
  if (importParser.parse(&txt, line, col, modules, &directives, &keepRanges))
  {
    for (auto& module : modules)
    {
      ezStringBuilder sb(module.moduleName.c_str());
      if (!sb.HasExtension(".spark"))
        continue;

      if (!ezFileSystem::ExistsFile(sb))
      {
        ezLog::Error("Imported file \"{}\" not found.", sb);
        continue;
      }

      ezStringBuilder file;
      if (ReadEntireFile(sb, file).Failed())
      {
        ezLog::Error("Cannot import file \"{}\".", sb);
        continue;
      }

      scriptCode.Append(file);
    }
  }

  scriptCode.Append("\n");
  scriptCode.Append(txt);

  if (m_pScriptContext->Run(scriptCode, &m_ComponentScope).Failed())
  {
    SetUserFlag(ScriptFlag::Failed, true);
    ezLog::Error("An error occurred while compiling the script.");
    return;
  }

  SetUserFlag(ScriptFlag::Compiled, true);

  m_ComponentInstance = Sqrat::Table(m_ComponentScope.GetSlot(_SC("Component")));

  if (!sq_isinstance(m_ComponentInstance.GetObject()))
  {
    ezLog::Error("Invalid script component. Missing the component instance.");
    return;
  }

  m_InitializeFunc = m_ComponentInstance.GetFunction(_SC("Initialize"));
  m_DeinitializeFunc = m_ComponentInstance.GetFunction(_SC("Deinitialize"));
  m_OnActivatedFunc = m_ComponentInstance.GetFunction(_SC("OnActivated"));
  m_OnDeactivatedFunc = m_ComponentInstance.GetFunction(_SC("OnDeactivated"));
  m_OnSimulationStarted = m_ComponentInstance.GetFunction(_SC("OnSimulationStarted"));
  m_UpdateFunc = m_ComponentInstance.GetFunction(_SC("Update"));

  sq_pushobject(m_ComponentScope.GetVM(), m_ComponentInstance.GetObject());
  sq_getclass(m_ComponentScope.GetVM(), -1);
  {
    const auto& componentClass = Sqrat::Var<Sqrat::Object>(m_ComponentScope.GetVM(), -1).value;

    sq_pushobject(m_ComponentScope.GetVM(), componentClass.GetObject());
    {
      sq_pushnull(m_ComponentScope.GetVM());
      if (SQ_SUCCEEDED(sq_getattributes(m_ComponentScope.GetVM(), -2)))
      {
        const auto& attributes = Sqrat::Var<Sqrat::Table>(m_ComponentScope.GetVM(), -1).value;
        if (!attributes.IsNull())
        {
          Sqrat::Object::iterator attIt;
          while (attributes.Next(attIt))
          {
            ezString attName = attIt.getName();

            if (attName.Compare("UpdateInterval") == 0)
            {
              const HSQOBJECT& attValue = attIt.getValue();

              if (sq_isnumeric(attValue))
              {
                SQFloat intervalMS = sq_objtofloat(&attValue);
                SetUpdateInterval(intervalMS);
              }
            }
          }
        }
      }
      sq_pop(m_ComponentScope.GetVM(), 1);

      Sqrat::Object::iterator it;
      while (m_ComponentInstance.Next(it))
      {
        sq_pushobject(m_ComponentScope.GetVM(), it.getKey());
        if (SQ_SUCCEEDED(sq_getattributes(m_ComponentScope.GetVM(), -2)))
        {
          const auto& attributes = Sqrat::Var<Sqrat::Table>(m_ComponentScope.GetVM(), -1).value;
          if (!attributes.IsNull())
          {
            Sqrat::Object::iterator attIt;
            while (attributes.Next(attIt))
            {
              ezString attName = attIt.getName();

              // Methods
              if (sq_isfunction(it.getValue()) || sq_isclosure(it.getValue()) || sq_isnativeclosure(it.getValue()))
              {
                if (attName.Compare("MessageHandler") == 0)
                {
                  const HSQOBJECT& attValue = attIt.getValue();

                  if (sq_isstring(attValue))
                  {
                    const SQChar* typeName = sq_objtostring(&attValue);
                    m_MessageHandlers.Insert(
                      ezHashingUtils::StringHash(typeName),
                      m_ComponentInstance.GetFunction(it.getName()));
                  }
                }
              }
              // Properties
              else
              {
                if (attName.Compare("Expose") == 0)
                {
                  // Expose property
                }
              }
            }
          }
        }
        sq_pop(m_ComponentScope.GetVM(), 1);
      }
    }
    sq_poptop(m_ComponentScope.GetVM());
  }
  sq_pop(m_ComponentScope.GetVM(), 2);

  if (!m_InitializeFunc.IsNull())
    m_InitializeFunc();

  // const auto& properties = m_ComponentScope.GetSlot("Properties");
  // if (properties.IsNull())
  //   return;
  //
  // Sqrat::Object::iterator it;
  // while (properties.Next(it))
  // {
  //   ezLog::Info("{} {}", it.getName(), sq_objtypestr(it.getValueType()));
  //   switch (it.getValueType())
  //   {
  //     case OT_BOOL:
  //     {
  //       auto* prop = new ezSparkLangScriptProperty_Bool(it.getName());
  //       const auto& value = it.getValue();
  //       prop->SetValue(sq_objtobool(&value));
  //       m_Properties.ExpandAndGetRef() = prop;
  //       break;
  //     }
  //     case OT_FLOAT:
  //     {
  //       auto* prop = new ezSparkLangScriptProperty_Float(it.getName());
  //       const auto& value = it.getValue();
  //       prop->SetValue(sq_objtofloat(&value));
  //       m_Properties.ExpandAndGetRef() = prop;
  //       break;
  //     }
  //     case OT_INTEGER:
  //     {
  //       auto* prop = new ezSparkLangScriptProperty_Integer(it.getName());
  //       const auto& value = it.getValue();
  //       prop->SetValue(sq_objtointeger(&value));
  //       m_Properties.ExpandAndGetRef() = prop;
  //       break;
  //     }
  //     case OT_STRING:
  //     {
  //       auto* prop = new ezSparkLangScriptProperty_String(it.getName());
  //       const auto& value = it.getValue();
  //       prop->SetValue(sq_objtostring(&value));
  //       m_Properties.ExpandAndGetRef() = prop;
  //       break;
  //     }
  //   }
  // }
}

void ezSparkLangScriptComponent::Deinitialize()
{
  if (!m_DeinitializeFunc.IsNull())
    m_DeinitializeFunc();

  Sqrat::RootTable root(m_pScriptContext->GetVM());
  root.DeleteSlot(GetUniqueID());

  EZ_DEFAULT_DELETE(m_pScriptContext);

  SUPER::Deinitialize();
}

void ezSparkLangScriptComponent::OnActivated()
{
  SUPER::OnActivated();

  if (!m_OnActivatedFunc.IsNull())
    m_OnActivatedFunc();
}

void ezSparkLangScriptComponent::OnDeactivated()
{
  if (!m_OnDeactivatedFunc.IsNull())
    m_OnDeactivatedFunc();

  SUPER::OnDeactivated();
}

void ezSparkLangScriptComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  EnableUnhandledMessageHandler(true);

  if (!m_OnSimulationStarted.IsNull())
    m_OnSimulationStarted();
}

void ezSparkLangScriptComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);
}

void ezSparkLangScriptComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
}

bool ezSparkLangScriptComponent::OnUnhandledMessage(ezMessage& msg, bool bWasPostedMsg)
{
  return HandleUnhandledMessage(msg, bWasPostedMsg);
}

bool ezSparkLangScriptComponent::OnUnhandledMessage(ezMessage& msg, bool bWasPostedMsg) const
{
  return const_cast<ezSparkLangScriptComponent*>(this)->HandleUnhandledMessage(msg, bWasPostedMsg);
}

bool ezSparkLangScriptComponent::HandlesEventMessage(const ezEventMessage& msg) const
{
  if (GetUserFlag(ScriptFlag::Failed))
    return false;

  // Native events
  if (const auto* pRtti = msg.GetDynamicRTTI())
    return m_MessageHandlers.Contains(pRtti->GetTypeNameHash());

  // Pure SparkLang events
  if (msg.IsInstanceOf<ezSparkLangScriptEventMessageProxy>())
    return m_MessageHandlers.Contains(ezStaticCast<const ezSparkLangScriptEventMessageProxy&>(msg).m_sMessageTypeNameHash);

  return false;
}

void ezSparkLangScriptComponent::Update()
{
  const ezTime now = ezTime::Now();

  if ((now - m_LastUpdateTime) < m_UpdateInterval)
    return;

  if (!m_UpdateFunc.IsNull())
    m_UpdateFunc();

  m_LastUpdateTime = now;
}

bool ezSparkLangScriptComponent::HandleUnhandledMessage(ezMessage& msg, bool bWasPostedMsg)
{
  if (GetUserFlag(ScriptFlag::Failed))
    return false;

  // Native messages
  if (const auto* pRtti = msg.GetDynamicRTTI(); m_MessageHandlers.Contains(pRtti->GetTypeNameHash()))
    return m_MessageHandlers[pRtti->GetTypeNameHash()].Execute(msg);

  // Pure SparkLang messages
  if (msg.IsInstanceOf<ezSparkLangScriptMessageProxy>())
  {
    const auto& evt = ezStaticCast<const ezSparkLangScriptMessageProxy&>(msg);
    return m_MessageHandlers[evt.m_sMessageTypeNameHash].Execute(evt.m_pMessage);
  }

  // Pure SparkLang events
  if (msg.IsInstanceOf<ezSparkLangScriptEventMessageProxy>())
  {
    const auto& evt = ezStaticCast<const ezSparkLangScriptEventMessageProxy&>(msg);
    return m_MessageHandlers[evt.m_sMessageTypeNameHash].Execute(evt.m_pEventMessage);
  }

  return false;
}

void ezSparkLangScriptComponent::OnMsgSparkLangScriptMessageProxy(ezSparkLangScriptMessageProxy& msg)
{
  if (GetUserFlag(ScriptFlag::Failed))
    return;

  if (const auto& it = m_MessageHandlers.Find(msg.m_sMessageTypeNameHash); it.IsValid())
    it.Value().Execute(msg.m_pMessage);
}
