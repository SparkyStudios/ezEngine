#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>
#include <SparkLangPlugin/Core/ScriptContext.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

#include <Foundation/Reflection/ReflectionUtils.h>

ezComponent* GetComponentFromVM(HSQUIRRELVM vm, SQInteger index)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return nullptr;
  }

  const auto& hComponent = Sqrat::Var<ezComponentHandle>(vm, index);

  if (hComponent.value.IsInvalidated())
    return nullptr;

  ezComponent* pComponent = nullptr;
  const bool bIsValid = pContext->GetWorld()->TryGetComponent(hComponent.value, pComponent);

  return bIsValid ? pComponent : nullptr;
}

// ez.Component.IsValid(integer): bool
SQInteger ezspComponentIsValid(HSQUIRRELVM vm)
{
  sq_pushbool(vm, GetComponentFromVM(vm, -1) != nullptr);
  return 1;
}

// ez.Component.GetUniqueID(integer): integer
SQInteger ezspComponentGetUniqueID(HSQUIRRELVM vm)
{
  if (const ezComponent* pComponent = GetComponentFromVM(vm, -1); pComponent != nullptr)
  {
    sq_pushinteger(vm, pComponent->GetUniqueID());
    return 1;
  }

  sq_pushnull(vm);
  return 1;
}

// ez.Component.GetOwner(integer): integer
SQInteger ezspComponentGetOwner(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm, -1); pComponent != nullptr)
  {
    const ezGameObject* pGameObject = pComponent->GetOwner();
    Sqrat::PushVar(vm, pGameObject->GetHandle());
    return 1;
  }

  sq_pushnull(vm);
  return 1;
}

// ez.Component.SetActiveFlag(integer, bool): void
SQInteger ezspComponentSetActiveFlag(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm, -2); pComponent != nullptr)
  {
    SQBool flag = false;
    if (SQ_SUCCEEDED(sq_getbool(vm, -1, &flag)))
      pComponent->SetActiveFlag(flag);
  }

  return 0;
}

// ez.Component.GetActiveFlag(integer): bool
// ez.Component.IsActive(integer): bool
// ez.Component.IsActiveAndInitialized(integer): bool
// ez.Component.IsActiveAndSimulating(integer): bool
template <int overload>
SQInteger ezspComponentGetActiveFlag(HSQUIRRELVM vm)
{
  if (const ezComponent* pComponent = GetComponentFromVM(vm, -1); pComponent != nullptr)
  {
    if constexpr (overload == 1)
    {
      sq_pushbool(vm, pComponent->GetActiveFlag());
    }
    else if constexpr (overload == 2)
    {
      sq_pushbool(vm, pComponent->IsActive());
    }
    else if constexpr (overload == 3)
    {
      sq_pushbool(vm, pComponent->IsActiveAndInitialized());
    }
    else if constexpr (overload == 4)
    {
      sq_pushbool(vm, pComponent->IsActiveAndSimulating());
    }
    else
    {
      EZ_ASSERT_NOT_IMPLEMENTED;
      sq_pushnull(vm);
    }
  }

  return 1;
}

// ez.Component.SendMessage(integer, ezMessage): void
// ez.Component.PostMessage(integer, ezMessage, integer): void
template <int overload>
SQInteger ezspComponentSendMessage(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm, 2); pComponent != nullptr)
  {
    sq_typeof(vm, 3);

    const SQChar* msgType;
    sq_getstring(vm, -1, &msgType);
    sq_poptop(vm);

    auto const message = Sqrat::Var<ezMessage*>(vm, 3);

    ezMessage* pMsg = message.value;

    if (const ezUInt64 uiHash = ezHashingUtils::StringHash(msgType); message.value->GetDynamicRTTI()->GetTypeNameHash() != uiHash)
    {
      pMsg = ezGetStaticRTTI<ezSparkLangScriptMessageProxy>()->GetAllocator()->Allocate<ezSparkLangScriptMessageProxy>();
      ezDynamicCast<ezSparkLangScriptMessageProxy*>(pMsg)->m_sMessageTypeNameHash = ezHashingUtils::StringHashTo32(uiHash);
      ezDynamicCast<ezSparkLangScriptMessageProxy*>(pMsg)->m_pMessage = message.value;
    }

    if constexpr (overload == 1)
    {
      pComponent->SendMessage(*pMsg);
    }
    else if constexpr (overload == 2)
    {
      SQFloat fMilliseconds = 0;
      sq_getfloat(vm, -1, &fMilliseconds);

      pComponent->PostMessage(*pMsg, ezTime::Milliseconds(fMilliseconds));
    }
    else
    {
      EZ_ASSERT_NOT_IMPLEMENTED;
    }
  }

  return 0;
}

// ez.Component.SetDebugOutput(integer, bool): void
// ez.Component.SetGlobalEventHandlerMode(integer, bool): void
// ez.Component.SetPassThroughUnhandledEvents(integer, bool): void
template <int overload>
SQInteger ezspComponentSetEventMessageHandlerComponentProp(HSQUIRRELVM vm)
{
  if (auto* pComponent = ezDynamicCast<ezEventMessageHandlerComponent*>(GetComponentFromVM(vm, -2)); pComponent != nullptr)
  {
    const Sqrat::Var<bool> var(vm, -1);

    if constexpr (overload == 1)
      pComponent->SetDebugOutput(var.value);
    else if constexpr (overload == 2)
      pComponent->SetGlobalEventHandlerMode(var.value);
    else
      pComponent->SetPassThroughUnhandledEvents(var.value);
  }

  return 0;
}

// ez.Component.GetDebugOutput(integer): bool
// ez.Component.GetGlobalEventHandlerMode(integer): bool
// ez.Component.GetPassThroughUnhandledEvents(integer): bool
template <int overload>
SQInteger ezspComponentGetEventMessageHandlerComponentProp(HSQUIRRELVM vm)
{
  const auto* pComponent = ezDynamicCast<ezEventMessageHandlerComponent*>(GetComponentFromVM(vm, -1));

  if (pComponent == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  if constexpr (overload == 1)
    Sqrat::PushVar(vm, pComponent->GetDebugOutput());
  else if constexpr (overload == 2)
    Sqrat::PushVar(vm, pComponent->GetGlobalEventHandlerMode());
  else
    Sqrat::PushVar(vm, pComponent->GetPassThroughUnhandledEvents());

  return 1;
}

// ez.Component.BroadcastEvent(integer, ezEventMessage): void
SQInteger ezspComponentBroadcastEvent(HSQUIRRELVM vm)
{
  if (auto* pComponent = ezDynamicCast<ezSparkLangScriptComponent*>(GetComponentFromVM(vm, -2)); pComponent != nullptr)
  {
    sq_typeof(vm, -1);

    const SQChar* msgType;
    sq_getstring(vm, -1, &msgType);
    sq_poptop(vm);

    auto const message = Sqrat::Var<ezEventMessage*>(vm, -1);

    ezEventMessage* pMsg = message.value;

    if (const ezUInt64 uiHash = ezHashingUtils::StringHash(msgType); message.value->GetDynamicRTTI()->GetTypeNameHash() != uiHash)
    {
      pMsg = ezGetStaticRTTI<ezSparkLangScriptEventMessageProxy>()->GetAllocator()->Allocate<ezSparkLangScriptEventMessageProxy>();
      ezDynamicCast<ezSparkLangScriptEventMessageProxy*>(pMsg)->m_sMessageTypeNameHash = ezHashingUtils::StringHashTo32(uiHash);
      ezDynamicCast<ezSparkLangScriptEventMessageProxy*>(pMsg)->m_pEventMessage = message.value;
    }

    pComponent->BroadcastEventMessage(*pMsg);
  }

  return 0;
}

// ez.Component.SetUpdateInterval(integer, float): void
SQInteger ezspComponentSetUpdateInterval(HSQUIRRELVM vm)
{
  if (auto* pComponent = ezDynamicCast<ezSparkLangScriptComponent*>(GetComponentFromVM(vm, -2)); pComponent != nullptr)
  {
    SQFloat fIntervalMs;
    sq_getfloat(vm, -1, &fIntervalMs);

    pComponent->SetUpdateInterval(fIntervalMs);
  }

  return 0;
}

// ez.Component.SetProp(integer, integer, any): void
SQInteger ezspComponentSetProp(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm, -3); pComponent != nullptr)
  {
    SQInteger uiPropHash;
    sq_getinteger(vm, -2, &uiPropHash);

    const ezSparkLangScriptContext::PropertyBinding* pBinding = ezSparkLangScriptContext::FindPropertyBinding(uiPropHash);

    if (pBinding == nullptr)
    {
      sq_throwerror(vm, _SC("Bound property not found"));
      return 0;
    }

    const ezVariant& value = GetVariantFromVM(vm, -1, pBinding->m_pProperty->GetSpecificType());

    ezReflectionUtils::SetMemberPropertyValue(pBinding->m_pProperty, pComponent, value);
  }

  return 0;
}

// ez.Component.GetProp(integer, integer): any
SQInteger ezspComponentGetProp(HSQUIRRELVM vm)
{
  const ezComponent* pComponent = GetComponentFromVM(vm, -2);

  if (pComponent == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  SQInteger uiPropHash;
  sq_getinteger(vm, -1, &uiPropHash);

  const ezSparkLangScriptContext::PropertyBinding* pBinding = ezSparkLangScriptContext::FindPropertyBinding(uiPropHash);

  if (pBinding == nullptr)
  {
    sq_throwerror(vm, _SC("Bound property not found"));
    return 0;
  }

  const ezVariant& value = ezReflectionUtils::GetMemberPropertyValue(pBinding->m_pProperty, pComponent);

  PushVariantToVM(vm, value);
  return 1;
}

// ez.Component.CallFunc(integer, integer, any...): any
SQInteger ezspComponentCallFunc(HSQUIRRELVM vm)
{
  ezComponent* pComponent = GetComponentFromVM(vm, 2);

  if (pComponent == nullptr)
    return 0;

  SQInteger uiPropHash;
  sq_getinteger(vm, 3, &uiPropHash);

  const ezSparkLangScriptContext::FunctionBinding* pBinding = ezSparkLangScriptContext::FindFunctionBinding(uiPropHash);

  if (pBinding == nullptr)
  {
    sq_throwerror(vm, _SC("Bound method not found."));
    return 0;
  }

  const ezUInt32 uiNumArgs = pBinding->m_pFunction->GetArgumentCount();
  if (uiNumArgs > sq_gettop(vm) - 3)
  {
    sq_throwerror(vm, _SC("Method called with an invalid number of arguments."));
    return 0;
  }

  ezVariant ret0;
  ezStaticArray<ezVariant, 16> args;
  args.SetCount(uiNumArgs);

  for (ezUInt32 arg = 0; arg < uiNumArgs; ++arg)
    args[arg] = GetVariantFromVM(vm, 4 + arg, pBinding->m_pFunction->GetArgumentType(arg));

  pBinding->m_pFunction->Execute(pComponent, args, ret0);

  if (pBinding->m_pFunction->GetReturnType() != nullptr)
  {
    PushVariantToVM(vm, ret0);
    return 1;
  }

  return 0;
}

SQRESULT ezSparkLangModule::ezComponent(Sqrat::Table& module)
{
  Sqrat::Table Component(module.GetVM());

  Sqrat::Enumeration ComponentMode(module.GetVM());
  ComponentMode.Const(_SC("Static"), ezComponentMode::Static);
  ComponentMode.Const(_SC("Dynamic"), ezComponentMode::Dynamic);

  Sqrat::Class<ezMessage> MessageClass(module.GetVM(), _SC("ezMessage"));
  MessageClass
    .Ctor()
    .Func(_SC("GetSortingKey"), &ezMessage::GetSortingKey)
    .Func(_SC("GetId"), &ezMessage::GetId)
    .Func(_SC("GetSize"), &ezMessage::GetSize)
    .Func(_SC("GetHash"), &ezMessage::GetHash);

  Sqrat::DerivedClass<ezEventMessage, ezMessage> EventMessageClass(module.GetVM(), _SC("ezEventMessage"));
  EventMessageClass
    .Ctor()
    .ConstVar(_SC("SenderObject"), &ezEventMessage::m_hSenderObject)
    .ConstVar(_SC("SenderComponent"), &ezEventMessage::m_hSenderComponent);

  Component
    .Bind(_SC("ComponentMode"), ComponentMode)
    .Bind(_SC("Message"), MessageClass)
    .Bind(_SC("EventMessage"), EventMessageClass)
    .SquirrelFunc(_SC("IsValid"), ezspComponentIsValid, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetUniqueID"), ezspComponentGetUniqueID, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetOwner"), ezspComponentGetOwner, 2, _SC(".i"))
    .SquirrelFunc(_SC("SetActiveFlag"), ezspComponentSetActiveFlag, 3, _SC(".ib"))
    .SquirrelFunc(_SC("GetActiveFlag"), ezspComponentGetActiveFlag<1>, 2, _SC(".i"))
    .SquirrelFunc(_SC("IsActive"), ezspComponentGetActiveFlag<2>, 2, _SC(".i"))
    .SquirrelFunc(_SC("IsActiveAndInitialized"), ezspComponentGetActiveFlag<3>, 2, _SC(".i"))
    .SquirrelFunc(_SC("IsActiveAndSimulating"), ezspComponentGetActiveFlag<4>, 2, _SC(".i"))
    .SquirrelFunc(_SC("SendMessage"), ezspComponentSendMessage<1>, 3, _SC(".ix"))
    .SquirrelFunc(_SC("PostMessage"), ezspComponentSendMessage<2>, 4, _SC(".ixf"))
    .SquirrelFunc(_SC("SetDebugOutput"), ezspComponentSetEventMessageHandlerComponentProp<1>, 3, _SC(".ib"))
    .SquirrelFunc(_SC("SetGlobalEventHandlerMode"), ezspComponentSetEventMessageHandlerComponentProp<2>, 3, _SC(".ib"))
    .SquirrelFunc(_SC("SetPassThroughUnhandledEvents"), ezspComponentSetEventMessageHandlerComponentProp<3>, 3, _SC(".ib"))
    .SquirrelFunc(_SC("GetDebugOutput"), ezspComponentGetEventMessageHandlerComponentProp<1>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalEventHandlerMode"), ezspComponentGetEventMessageHandlerComponentProp<2>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetPassThroughUnhandledEvents"), ezspComponentGetEventMessageHandlerComponentProp<3>, 2, _SC(".i"))
    .SquirrelFunc(_SC("BroadcastEvent"), ezspComponentBroadcastEvent, 3, _SC(".ix"))
    .SquirrelFunc(_SC("SetUpdateInterval"), ezspComponentSetUpdateInterval, 3, _SC(".if"))
    .SquirrelFunc(_SC("SetProp"), ezspComponentSetProp, 4, _SC(".ii."))
    .SquirrelFunc(_SC("GetProp"), ezspComponentGetProp, 3, _SC(".ii"))
    .SquirrelFunc(_SC("CallFunc"), ezspComponentCallFunc, -3, _SC(".ii"));

  module
    .Bind(_SC("Component"), Component);

  return SQ_OK;
}
