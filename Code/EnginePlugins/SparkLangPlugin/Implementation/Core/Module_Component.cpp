#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>
#include <SparkLangPlugin/Core/ScriptContext.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

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

  sq_pushinteger(vm, 0);
  return 1;
}

// ez.Component.GetOwner(integer): userpointer
SQInteger ezspComponentGetOwner(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm, -1); pComponent != nullptr)
  {
    sq_pushuserpointer(vm, pComponent->GetOwner());
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
template <int type>
SQInteger ezspComponentGetActiveFlag(HSQUIRRELVM vm)
{
  if (const ezComponent* pComponent = GetComponentFromVM(vm, -1); pComponent != nullptr)
  {
    switch (type)
    {
      case 1:
        sq_pushbool(vm, pComponent->GetActiveFlag());
        break;
      case 2:
        sq_pushbool(vm, pComponent->IsActive());
        break;
      case 3:
        sq_pushbool(vm, pComponent->IsActiveAndInitialized());
        break;
      case 4:
        sq_pushbool(vm, pComponent->IsActiveAndSimulating());
        break;
    }
  }

  return 1;
}

// ez.Component.SendMessage(integer, ezMessage): void
// ez.Component.PostMessage(integer, ezMessage, integer): void
template <int type>
SQInteger ezspComponentSendMessage(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm, -type - 1); pComponent != nullptr)
  {
    sq_typeof(vm, -type);

    const SQChar* msgType;
    sq_getstring(vm, -type, &msgType);
    sq_poptop(vm);

    auto const message = Sqrat::Var<ezMessage*>(vm, -type);

    ezSparkLangScriptMessageProxy* pMsg = ezGetStaticRTTI<ezSparkLangScriptMessageProxy>()->GetAllocator()->Allocate<ezSparkLangScriptMessageProxy>();
    pMsg->m_sMessageTypeNameHash = ezHashingUtils::StringHash(msgType);
    pMsg->m_pMessage = message.value;

    switch (type)
    {
      case 1:
      {
        pComponent->SendMessage(*pMsg);
        break;
      }

      case 2:
      {
        SQFloat fMilliseconds = 0;
        sq_getfloat(vm, -1, &fMilliseconds);

        pComponent->PostMessage(*pMsg, ezTime::Milliseconds(fMilliseconds));
        break;
      }
    }
  }

  return 0;
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

    ezSparkLangScriptEventMessageProxy* pMsg = ezGetStaticRTTI<ezSparkLangScriptEventMessageProxy>()->GetAllocator()->Allocate<ezSparkLangScriptEventMessageProxy>();
    pMsg->m_sMessageTypeNameHash = ezHashingUtils::StringHash(msgType);
    pMsg->m_pEventMessage = message.value;

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
  MessageClass
    .Ctor()
    .Func(_SC("GetSortingKey"), &ezEventMessage::GetSortingKey)
    .Func(_SC("GetId"), &ezEventMessage::GetId)
    .Func(_SC("GetSize"), &ezEventMessage::GetSize)
    .Func(_SC("GetHash"), &ezEventMessage::GetHash);

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
    .SquirrelFunc(_SC("BroadcastEvent"), ezspComponentBroadcastEvent, 3, _SC(".ix"))
    .SquirrelFunc(_SC("SetUpdateInterval"), ezspComponentSetUpdateInterval, 3, _SC(".if"));

  module
    .Bind(_SC("Component"), Component);

  return SQ_OK;
}
