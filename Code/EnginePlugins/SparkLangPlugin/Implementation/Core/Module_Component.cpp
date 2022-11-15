#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>
#include <SparkLangPlugin/Core/ScriptContext.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

ezComponent* GetComponentFromVM(HSQUIRRELVM vm)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return nullptr;
  }

  const Sqrat::RootTable root(vm);
  const auto& hComponent = root.GetSlotValue<ezComponentHandle>(_SC("componentId"), ezComponentHandle());

  if (hComponent.IsInvalidated())
    return nullptr;

  ezComponent* pComponent = nullptr;
  const bool bIsValid = pContext->GetWorld()->TryGetComponent(hComponent, pComponent);

  return bIsValid ? pComponent : nullptr;
}

// ez.Component.IsValid(): bool
SQInteger ezspComponentIsValid(HSQUIRRELVM vm)
{
  sq_pushbool(vm, GetComponentFromVM(vm) != nullptr);
  return 1;
}

// ez.Component.GetUniqueID(): integer
SQInteger ezspComponentGetUniqueID(HSQUIRRELVM vm)
{
  if (const ezComponent* pComponent = GetComponentFromVM(vm); pComponent != nullptr)
  {
    sq_pushinteger(vm, pComponent->GetUniqueID());
    return 1;
  }

  sq_pushinteger(vm, 0);
  return 1;
}

// ez.Component.GetOwner(): userpointer
SQInteger ezspComponentGetOwner(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm); pComponent != nullptr)
  {
    sq_pushuserpointer(vm, pComponent->GetOwner());
    return 1;
  }

  sq_pushnull(vm);
  return 1;
}

// ez.Component.SetActiveFlag(bool): void
SQInteger ezspComponentSetActiveFlag(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm); pComponent != nullptr)
  {
    SQBool flag = false;
    if (SQ_SUCCEEDED(sq_getbool(vm, -1, &flag)))
      pComponent->SetActiveFlag(flag);
  }

  return 0;
}

// ez.Component.GetActiveFlag(): bool
// ez.Component.IsActive(): bool
// ez.Component.IsActiveAndInitialized(): bool
// ez.Component.IsActiveAndSimulating(): bool
template <int type>
SQInteger ezspComponentGetActiveFlag(HSQUIRRELVM vm)
{
  if (const ezComponent* pComponent = GetComponentFromVM(vm); pComponent != nullptr)
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

// ez.Component.SendMessage(ezMessage): void
// ez.Component.PostMessage(ezMessage, integer): void
template <int type>
SQInteger ezspComponentSendMessage(HSQUIRRELVM vm)
{
  if (ezComponent* pComponent = GetComponentFromVM(vm); pComponent != nullptr)
  {
    sq_typeof(vm, -type);

    const SQChar* msgType;
    sq_getstring(vm, -type, &msgType);
    sq_pop(vm, 1);

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

// ez.Component.BroadcastEvent(ezEventMessage): void
SQInteger ezspComponentBroadcastEvent(HSQUIRRELVM vm)
{
  if (auto* pComponent = ezDynamicCast<ezSparkLangScriptComponent*>(GetComponentFromVM(vm)); pComponent != nullptr)
  {
    sq_typeof(vm, -1);

    const SQChar* msgType;
    sq_getstring(vm, -1, &msgType);
    sq_pop(vm, 1);

    auto const message = Sqrat::Var<ezEventMessage*>(vm, -1);

    ezSparkLangScriptEventMessageProxy* pMsg = ezGetStaticRTTI<ezSparkLangScriptEventMessageProxy>()->GetAllocator()->Allocate<ezSparkLangScriptEventMessageProxy>();
    pMsg->m_sMessageTypeNameHash = ezHashingUtils::StringHash(msgType);
    pMsg->m_pEventMessage = message.value;

    pComponent->BroadcastEventMessage(*pMsg);
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
    .SquirrelFunc(_SC("IsValid"), ezspComponentIsValid, 1, _SC("."))
    .SquirrelFunc(_SC("GetUniqueID"), ezspComponentGetUniqueID, 1, _SC("."))
    .SquirrelFunc(_SC("GetOwner"), ezspComponentGetOwner, 1, _SC("."))
    .SquirrelFunc(_SC("SetActiveFlag"), ezspComponentSetActiveFlag, 2, _SC(".b"))
    .SquirrelFunc(_SC("GetActiveFlag"), ezspComponentGetActiveFlag<1>, 1, _SC("."))
    .SquirrelFunc(_SC("IsActive"), ezspComponentGetActiveFlag<2>, 1, _SC("."))
    .SquirrelFunc(_SC("IsActiveAndInitialized"), ezspComponentGetActiveFlag<3>, 1, _SC("."))
    .SquirrelFunc(_SC("IsActiveAndSimulating"), ezspComponentGetActiveFlag<4>, 1, _SC("."))
    .SquirrelFunc(_SC("SendMessage"), ezspComponentSendMessage<1>, 2, _SC(".x"))
    .SquirrelFunc(_SC("PostMessage"), ezspComponentSendMessage<2>, 3, _SC(".xf"))
    .SquirrelFunc(_SC("BroadcastEvent"), ezspComponentBroadcastEvent, 2, _SC("."))
  ;

  module
    .Bind(_SC("Component"), Component);

  return SQ_OK;
}
