#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

#include <sqrat.h>
#include <squirrel.h>

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
let ez = require("ez")

let class ezComponent {
  constructor() {}

  function IsValid() {
    return ez.Component.IsValid()
  }

  function GetUniqueId() {
    return ez.Component.GetUniqueId()
  }

  function GetOwner() {
    return ez.Component.GetOwner()
  }

  function SetActiveFlag(active) {
    ez.Component.SetActiveFlag(active)
  }

  function GetActiveFlag() {
    return ez.Component.GetActiveFlag()
  }

  function IsActive() {
    return ez.Component.IsActive()
  }

  function IsActiveAndInitialized() {
    return ez.Component.IsActiveAndInitialized()
  }

  function IsActiveAndSimulating() {
    return ez.Component.IsActiveAndSimulating()
  }

  function SendMessage(message) {
    ez.Component.SendMessage(message)
  }

  function PostMessage(message) {
    ez.Component.PostMessage(message)
  }

  function BroadcastEvent(message) {
    ez.Component.BroadcastEvent(message)
  }
}

let ezMessage = ez.Component.Message

let ezEventMessage = ez.Component.EventMessage

let class TestMessage extends ezMessage {
  damage = ez.Component.GetUniqueID()
  entity = "npc"

  function _typeof() {
    return "TestMessage"
  }
}

let class TestEvent extends ezEventMessage {
  hit = ez.Component.GetUniqueID()

  function _typeof() {
    return "TestEvent"
  }
}

local once = false

let class TestComponent extends ezComponent {
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
    ez.Log.Success($"Activated {ez.Component.GetUniqueID()}")
  }

  function OnDeactivated() {
    ez.Log.Success("Deactivated")
  }

  function OnSimulationStarted() {
    ez.Log.Success("SimulationStarted")
  }

  function Update() {
    Count++
    ez.Log.Success($"Update {Name} {Count}")

    if (!once) {
      let msg = TestMessage()
      ez.Log.Info($"Sending {msg} {typeof msg} {msg.entity} {msg instanceof TestMessage}")
      ez.Component.SendMessage(msg)

      let event = TestEvent()
      ez.Log.Info($"Sending {event} {typeof event} {event.hit} {event instanceof TestEvent}")
      ez.Component.BroadcastEvent(event)

      once = true
    }
  }

  </ Handler = "TestMessage" />
  function OnTestMessage(msg) {
    if (msg instanceof TestMessage) {
      ez.Log.Success($"Seriously got a TestMessage from C++ {msg.damage}")
    }
  }

  </ Handler = "TestEvent" />
  function OnTestEvent(evt) {
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

void ezSparkLangScriptComponent::Initialize()
{
  SUPER::Initialize();

  m_pScriptContext = EZ_DEFAULT_NEW(ezSparkLangScriptContext, GetWorld());

  Sqrat::RootTable root(m_pScriptContext->GetVM());
  root.SetValue<const ezComponentHandle&>(_SC("componentId"), GetHandle());

  m_ComponentScope = Sqrat::Table(m_pScriptContext->GetVM());
  root.SetValue(GetUniqueID(), m_ComponentScope);

  if (m_pScriptContext->Run(Test, &m_ComponentScope).Failed())
  {
    SetUserFlag(ScriptFlag::Failed, true);
    ezLog::Error("An error occurred while compiling the script.");
    return;
  }

  SetUserFlag(ScriptFlag::Compiled, true);

  // m_InitializeFunc = m_ComponentScope.GetFunction(_SC("Initialize"));
  // m_DeinitializeFunc = m_ComponentScope.GetFunction(_SC("Deinitialize"));
  // m_OnActivatedFunc = m_ComponentScope.GetFunction(_SC("OnActivated"));
  // m_OnDeactivatedFunc = m_ComponentScope.GetFunction(_SC("OnDeactivated"));
  // m_OnSimulationStarted = m_ComponentScope.GetFunction(_SC("OnSimulationStarted"));
  // m_UpdateFunc = m_ComponentScope.GetFunction(_SC("Update"));
  //
  // if (auto messageHandlers = m_ComponentScope.GetSlot("MessageHandlers"); !messageHandlers.IsNull())
  // {
  //   Sqrat::Object::iterator it;
  //   while (messageHandlers.Next(it))
  //   {
  //     // We can't register non callable values
  //     if (!sq_isclosure(it.getValue()))
  //       continue;
  //
  //     m_MessageHandlers.Insert(
  //       ezHashingUtils::StringHash(it.getName()),
  //       Sqrat::Function(m_pScriptContext->GetVM(), messageHandlers, it.getValue()));
  //   }
  // }

  auto component = Sqrat::Table(m_ComponentScope.GetSlot(_SC("Component")));

  if (!sq_isinstance(component.GetObject()))
  {
    ezLog::Error("Invalid script component. Missing the component instance.");
    return;
  }

  m_InitializeFunc = component.GetFunction(_SC("Initialize"));
  m_DeinitializeFunc = component.GetFunction(_SC("Deinitialize"));
  m_OnActivatedFunc = component.GetFunction(_SC("OnActivated"));
  m_OnDeactivatedFunc = component.GetFunction(_SC("OnDeactivated"));
  m_OnSimulationStarted = component.GetFunction(_SC("OnSimulationStarted"));
  m_UpdateFunc = component.GetFunction(_SC("Update"));

  sq_pushobject(m_ComponentScope.GetVM(), component.GetObject());
  sq_getclass(m_ComponentScope.GetVM(), -1);

  auto componentClass = Sqrat::Var<Sqrat::Object>(m_ComponentScope.GetVM(), -1).value;

  Sqrat::Object::iterator it;
  while (componentClass.Next(it))
  {
    sq_pushobject(m_ComponentScope.GetVM(), componentClass.GetObject());
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
            if (attName.Compare("Handler") == 0)
            {
              const HSQOBJECT& attValue = attIt.getValue();

              if (sq_isstring(attValue))
              {
                const SQChar* typeName = sq_objtostring(&attValue);
                m_MessageHandlers.Insert(
                  ezHashingUtils::StringHash(typeName),
                  Sqrat::Function(m_pScriptContext->GetVM(), componentClass, it.getValue()));
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
    sq_pop(m_ComponentScope.GetVM(), 2);
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
  if (const auto* pRtti = msg.GetDynamicRTTI(); m_MessageHandlers.Contains(pRtti->GetTypeNameHash()))
    return true;

  // Pure SparkLang events
  if (msg.IsInstanceOf<ezSparkLangScriptEventMessageProxy>())
    return m_MessageHandlers.Contains(ezStaticCast<const ezSparkLangScriptEventMessageProxy&>(msg).m_sMessageTypeNameHash);

  return false;
}

void ezSparkLangScriptComponent::Update()
{
  if (!m_UpdateFunc.IsNull())
    m_UpdateFunc();
}

bool ezSparkLangScriptComponent::HandleUnhandledMessage(ezMessage& msg, bool bWasPostedMsg)
{
  if (GetUserFlag(ScriptFlag::Failed))
    return false;

  // Native events
  if (const auto* pRtti = msg.GetDynamicRTTI(); m_MessageHandlers.Contains(pRtti->GetTypeNameHash()))
    return m_MessageHandlers[pRtti->GetTypeNameHash()].Execute(msg);

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
