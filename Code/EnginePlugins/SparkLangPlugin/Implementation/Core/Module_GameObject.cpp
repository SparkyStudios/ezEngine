#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>
#include <SparkLangPlugin/Core/ScriptContext.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

namespace GameObjectProp
{
  enum Enum
  {
    LocalPosition,
    GlobalPosition,
    LocalScaling,
    GlobalScaling,
    LocalUniformScaling,
    LocalRotation,
    GlobalRotation,
    GlobalDirForwards,
    GlobalDirRight,
    GlobalDirUp,
    Velocity,
    ActiveFlag,
    Active,
    Dynamic,
    Static,
    ChildCount,
    SetParent,
    AddChild,
    DetachChild,
    Name,
    GlobalKey,
  };
}

ezGameObject* GetGameObjectFromVM(HSQUIRRELVM vm, SQInteger index)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return nullptr;
  }

  const auto& hGameObject = Sqrat::Var<ezGameObjectHandle>(vm, index);

  if (hGameObject.value.IsInvalidated())
    return nullptr;

  ezGameObject* pGameObject = nullptr;
  const bool bIsValid = pContext->GetWorld()->TryGetObject(hGameObject.value, pGameObject);

  return bIsValid ? pGameObject : nullptr;
}

// ez.GameObject.IsValid(integer): bool
SQInteger ezspGameObjectIsValid(HSQUIRRELVM vm)
{
  sq_pushbool(vm, GetGameObjectFromVM(vm, -1) != nullptr);
  return 1;
}

// ez.GameObject.SetLocalPosition(integer, ezVec3): void
// ez.GameObject.SetGlobalPosition(integer,ezVec3): void
// ez.GameObject.SetLocalScaling(integer,ezVec3): void
// ez.GameObject.SetGlobalScaling(integer,ezVec3): void
// ez.GameObject.SetVelocity(integer,ezVec3): void
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectSetVec3Prop(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2); pGameObject != nullptr)
  {
    if (!pGameObject->IsDynamic())
    {
      ezLog::SeriousWarning(
        "SparkLang component modifies transform of static game-object '{}'. Use 'Force Dynamic' mode on owner game-object.", pGameObject->GetName());

      pGameObject->MakeDynamic();
    }

    const auto& vec3 = Sqrat::Var<ezVec3>(vm, -1);

    switch (property)
    {
      case GameObjectProp::LocalPosition:
      {
        pGameObject->SetLocalPosition(vec3.value);
        break;
      }
      case GameObjectProp::GlobalPosition:
      {
        pGameObject->SetGlobalPosition(vec3.value);
        break;
      }
      case GameObjectProp::LocalScaling:
      {
        pGameObject->SetLocalScaling(vec3.value);
        break;
      }
      case GameObjectProp::GlobalScaling:
      {
        pGameObject->SetGlobalScaling(vec3.value);
        break;
      }
      case GameObjectProp::Velocity:
      {
        pGameObject->SetVelocity(vec3.value);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.GetLocalPosition(integer): ezVec3
// ez.GameObject.GetGlobalPosition(integer): ezVec3
// ez.GameObject.GetLocalScaling(integer): ezVec3
// ez.GameObject.GetGlobalScaling(integer): ezVec3
// ez.GameObject.GetGlobalDirUp(integer): ezVec3
// ez.GameObject.GetGlobalDirRight(integer): ezVec3
// ez.GameObject.GetGlobalDirForwards(integer): ezVec3
// ez.GameObject.GetVelocity(integer): ezVec3
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectGetVec3Prop(HSQUIRRELVM vm)
{
  if (const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1); pGameObject != nullptr)
  {
    switch (property)
    {
      case GameObjectProp::LocalPosition:
      {
        const auto& vec3 = pGameObject->GetLocalPosition();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      case GameObjectProp::GlobalPosition:
      {
        const auto& vec3 = pGameObject->GetGlobalPosition();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      case GameObjectProp::LocalScaling:
      {
        const auto& vec3 = pGameObject->GetLocalScaling();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      case GameObjectProp::GlobalScaling:
      {
        const auto& vec3 = pGameObject->GetGlobalScaling();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      case GameObjectProp::GlobalDirUp:
      {
        const auto& vec3 = pGameObject->GetGlobalDirUp();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      case GameObjectProp::GlobalDirRight:
      {
        const auto& vec3 = pGameObject->GetGlobalDirRight();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      case GameObjectProp::GlobalDirForwards:
      {
        const auto& vec3 = pGameObject->GetGlobalDirForwards();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      case GameObjectProp::Velocity:
      {
        const auto& vec3 = pGameObject->GetVelocity();
        Sqrat::PushVar(vm, vec3);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        sq_pushnull(vm);
        break;
      }
    }
  }

  return 1;
}

// ez.GameObject.SetLocalUniformScaling(integer, float): void
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectSetFloatProp(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2); pGameObject != nullptr)
  {
    if (!pGameObject->IsDynamic())
    {
      ezLog::SeriousWarning(
        "SparkLang component modifies transform of static game-object '{}'. Use 'Force Dynamic' mode on owner game-object.", pGameObject->GetName());

      pGameObject->MakeDynamic();
    }

    const auto& value = Sqrat::Var<float>(vm, -1);

    switch (property)
    {
      case GameObjectProp::LocalUniformScaling:
      {
        pGameObject->SetLocalUniformScaling(value.value);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.GetLocalUniformScaling(integer): float
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectGetFloatProp(HSQUIRRELVM vm)
{
  if (const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1); pGameObject != nullptr)
  {
    switch (property)
    {
      case GameObjectProp::LocalUniformScaling:
      {
        const auto& value = pGameObject->GetLocalUniformScaling();
        Sqrat::PushVar(vm, value);
        break;
      }
      case GameObjectProp::ChildCount:
      {
        const auto& value = pGameObject->GetChildCount();
        Sqrat::PushVar(vm, value);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        sq_pushnull(vm);
        break;
      }
    }
  }

  return 1;
}

// ez.GameObject.SetLocalRotation(integer, ezQuat): void
// ez.GameObject.SetGlobalRotation(integer, ezQuat): void
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectSetQuatProp(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2); pGameObject != nullptr)
  {
    if (!pGameObject->IsDynamic())
    {
      ezLog::SeriousWarning(
        "SparkLang component modifies transform of static game-object '{}'. Use 'Force Dynamic' mode on owner game-object.", pGameObject->GetName());

      pGameObject->MakeDynamic();
    }

    const auto& value = Sqrat::Var<ezQuat>(vm, -1);

    switch (property)
    {
      case GameObjectProp::LocalRotation:
      {
        pGameObject->SetLocalRotation(value.value);
        break;
      }
      case GameObjectProp::GlobalRotation:
      {
        pGameObject->SetGlobalRotation(value.value);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.GetLocalRotation(integer): ezQuat
// ez.GameObject.GetGlobalRotation(integer): ezQuat
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectGetQuatProp(HSQUIRRELVM vm)
{
  if (const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1); pGameObject != nullptr)
  {
    switch (property)
    {
      case GameObjectProp::LocalRotation:
      {
        const auto& value = pGameObject->GetLocalRotation();
        Sqrat::PushVar(vm, value);
        break;
      }
      case GameObjectProp::GlobalRotation:
      {
        const auto& value = pGameObject->GetGlobalRotation();
        Sqrat::PushVar(vm, value);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        sq_pushnull(vm);
        break;
      }
    }
  }

  return 1;
}

// ez.GameObject.SetActiveFlag(integer, bool): void
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectSetBoolProp(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2); pGameObject != nullptr)
  {
    const auto& value = Sqrat::Var<bool>(vm, -1);

    switch (property)
    {
      case GameObjectProp::ActiveFlag:
      {
        pGameObject->SetActiveFlag(value.value);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.GetActiveFlag(integer): bool
// ez.GameObject.IsActive(integer): bool
// ez.GameObject.IsDynamic(integer): bool
// ez.GameObject.IsStatic(integer): bool
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectGetBoolProp(HSQUIRRELVM vm)
{
  if (const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1); pGameObject != nullptr)
  {
    switch (property)
    {
      case GameObjectProp::ActiveFlag:
      {
        const auto& value = pGameObject->GetActiveFlag();
        Sqrat::PushVar(vm, value);
        break;
      }
      case GameObjectProp::Active:
      {
        const auto& value = pGameObject->IsActive();
        Sqrat::PushVar(vm, value);
        break;
      }
      case GameObjectProp::Dynamic:
      {
        const auto& value = pGameObject->IsDynamic();
        Sqrat::PushVar(vm, value);
        break;
      }
      case GameObjectProp::Static:
      {
        const auto& value = pGameObject->IsStatic();
        Sqrat::PushVar(vm, value);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        sq_pushnull(vm);
        break;
      }
    }
  }

  return 1;
}

// ez.GameObject.FindChildByName(integer, string, bool): integer
SQInteger ezspGameObjectFindChildByName(HSQUIRRELVM vm)
{
  ezGameObject* pGameObject = GetGameObjectFromVM(vm, -3);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const SQChar* szName;
  sq_getstring(vm, -2, &szName);

  SQBool bRecursive;
  sq_getbool(vm, -1, &bRecursive);

  if (const ezGameObject* pChild = pGameObject->FindChildByName(ezTempHashedString(szName), bRecursive); pChild != nullptr)
    Sqrat::PushVar(vm, pChild->GetHandle());
  else
    sq_pushnull(vm);

  return 1;
}

// ez.GameObject.FindChildByPath(integer, string): integer
SQInteger ezspGameObjectFindChildByPath(HSQUIRRELVM vm)
{
  ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const SQChar* szPath;
  sq_getstring(vm, -1, &szPath);

  if (const ezGameObject* pChild = pGameObject->FindChildByPath(szPath); pChild != nullptr)
    Sqrat::PushVar(vm, pChild->GetHandle());
  else
    sq_pushnull(vm);

  return 1;
}

// ez.GameObject.TryGetComponentOfBaseType(integer, integer): integer
SQInteger ezspGameObjectTryGetComponentOfBaseType(HSQUIRRELVM vm)
{
  const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const Sqrat::Var<ezUInt32> hash(vm, -1);
  const ezRTTI* pRtti = ezRTTI::FindTypeByNameHash32(hash.value);

  if (pRtti == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const ezComponent* pComponent = nullptr;
  if (!pGameObject->TryGetComponentOfBaseType(pRtti, pComponent))
  {
    sq_pushnull(vm);
    return 1;
  }

  Sqrat::PushVar(vm, pComponent->GetHandle());
  return 1;
}

// ez.GameObject.TryGetComponentsOfBaseType(integer, integer): integer[]
SQInteger ezspGameObjectTryGetComponentsOfBaseType(HSQUIRRELVM vm)
{
  const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const Sqrat::Var<ezUInt32> hash(vm, -1);
  const ezRTTI* pRtti = ezRTTI::FindTypeByNameHash32(hash.value);

  if (pRtti == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  ezDynamicArray<const ezComponent*> components;
  pGameObject->TryGetComponentsOfBaseType(pRtti, components);

  Sqrat::Array handles(vm);
  for (const auto& component : components)
    handles.Append(component->GetHandle());

  Sqrat::PushVar(vm, handles);
  return 1;
}

// ez.GameObject.SearchForChildByNameSequence(integer, string, string): integer
SQInteger ezspGameObjectSearchForChildByNameSequence(HSQUIRRELVM vm)
{
  ezGameObject* pGameObject = GetGameObjectFromVM(vm, -3);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const SQChar* szSearchPath;
  sq_getstring(vm, -2, &szSearchPath);

  const SQChar* szComponentType;
  SQInteger iSize = 0;
  sq_getstringandsize(vm, -1, &szComponentType, &iSize);

  const ezRTTI* pRtti = nullptr;

  if (iSize > 0)
    pRtti = ezRTTI::FindTypeByName(szComponentType);

  ezGameObject* pChild = pGameObject->SearchForChildByNameSequence(szSearchPath, pRtti);

  if (pChild == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  Sqrat::PushVar(vm, pChild->GetHandle());
  return 1;
}

// ez.GameObject.SearchForChildrenByNameSequence(integer, string, string): integer[]
SQInteger ezspGameObjectSearchForChildrenByNameSequence(HSQUIRRELVM vm)
{
  ezGameObject* pGameObject = GetGameObjectFromVM(vm, -3);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const SQChar* szSearchPath;
  sq_getstring(vm, -2, &szSearchPath);

  const SQChar* szComponentType;
  SQInteger iSize = 0;
  sq_getstringandsize(vm, -1, &szComponentType, &iSize);

  const ezRTTI* pRtti = nullptr;

  if (iSize > 0)
    pRtti = ezRTTI::FindTypeByName(szComponentType);

  ezHybridArray<ezGameObject*, 8> gameObjects;
  pGameObject->SearchForChildrenByNameSequence(szSearchPath, pRtti, gameObjects);

  Sqrat::Array handles(vm);
  for (const auto& gameObject : gameObjects)
    handles.Append(gameObject->GetHandle());

  Sqrat::PushVar(vm, handles);
  return 1;
}

// ez.GameObject.SendMessage(integer, ezMessage, bool): void
// ez.GameObject.PostMessage(integer, ezMessage, bool, float): void
template <int overload>
SQInteger ezspGameObjectSendMessage(HSQUIRRELVM vm)
{
  if (const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -overload - 2); pGameObject != nullptr)
  {
    sq_typeof(vm, -overload - 1);

    const SQChar* msgType;
    sq_getstring(vm, -1, &msgType);
    sq_poptop(vm);

    auto const message = Sqrat::Var<ezMessage*>(vm, -overload - 1);

    ezMessage* pMsg = message.value;

    if (const ezUInt64 uiHash = ezHashingUtils::StringHash(msgType); message.value->GetDynamicRTTI()->GetTypeNameHash() != uiHash)
    {
      pMsg = ezGetStaticRTTI<ezSparkLangScriptMessageProxy>()->GetAllocator()->Allocate<ezSparkLangScriptMessageProxy>();
      ezDynamicCast<ezSparkLangScriptMessageProxy*>(pMsg)->m_sMessageTypeNameHash = ezHashingUtils::StringHashTo32(uiHash);
      ezDynamicCast<ezSparkLangScriptMessageProxy*>(pMsg)->m_pMessage = message.value;
    }

    SQBool bRecursive;
    sq_getbool(vm, -overload, &bRecursive);

    switch (overload)
    {
      case 1:
      {
        if (bRecursive)
          pGameObject->SendMessageRecursive(*pMsg);
        else
          pGameObject->SendMessage(*pMsg);
        break;
      }
      case 2:
      {
        SQFloat fDelayMs;
        sq_getfloat(vm, -1, &fDelayMs);

        if (bRecursive)
          pGameObject->PostMessageRecursive(*pMsg, ezTime::Milliseconds(fDelayMs));
        else
          pGameObject->PostMessage(*pMsg, ezTime::Milliseconds(fDelayMs));
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.SendEventMessage(integer, ezEventMessage, integer): void
// ez.GameObject.PostEventMessage(integer, ezEventMessage, integer, float): void
template <int overload>
SQInteger ezspGameObjectSendEventMessage(HSQUIRRELVM vm)
{
  if (const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -overload - 2); pGameObject != nullptr)
  {
    sq_typeof(vm, -overload - 1);

    const SQChar* msgType;
    sq_getstring(vm, -1, &msgType);
    sq_poptop(vm);

    auto const message = Sqrat::Var<ezEventMessage*>(vm, -overload - 1);

    ezEventMessage* pMsg = message.value;

    if (const ezUInt64 uiHash = ezHashingUtils::StringHash(msgType); message.value->GetDynamicRTTI()->GetTypeNameHash() != uiHash)
    {
      pMsg = ezGetStaticRTTI<ezSparkLangScriptEventMessageProxy>()->GetAllocator()->Allocate<ezSparkLangScriptEventMessageProxy>();
      ezDynamicCast<ezSparkLangScriptEventMessageProxy*>(pMsg)->m_sMessageTypeNameHash = ezHashingUtils::StringHashTo32(uiHash);
      ezDynamicCast<ezSparkLangScriptEventMessageProxy*>(pMsg)->m_pEventMessage = message.value;
    }

    const ezComponent* pComponent;
    if (pComponent = GetComponentFromVM(vm, -overload); pComponent == nullptr)
    {
      sq_throwerror(vm, _SC("No component with the given handle has been found."));
      return 0;
    }

    switch (overload)
    {
      case 1:
      {
        pGameObject->SendEventMessage(*pMsg, pComponent);
        break;
      }
      case 2:
      {
        SQFloat fDelayMs;
        sq_getfloat(vm, -1, &fDelayMs);

        pGameObject->PostEventMessage(*pMsg, pComponent, ezTime::Milliseconds(fDelayMs));
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.SetName(integer, string): void
// ez.GameObject.SetGlobalKey(integer, string): void
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectSetStringProp(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2); pGameObject != nullptr)
  {
    const SQChar* szValue;
    sq_getstring(vm, -1, &szValue);

    switch (property)
    {
      case GameObjectProp::Name:
      {
        pGameObject->SetName(szValue);
        break;
      }
      case GameObjectProp::GlobalKey:
      {
        pGameObject->SetGlobalKey(szValue);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.GetName(integer): string
// ez.GameObject.GetGlobalKey(integer): string
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectGetStringProp(HSQUIRRELVM vm)
{
  const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1);
  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  switch (property)
  {
    case GameObjectProp::Name:
    {
      Sqrat::PushVar<const char*>(vm, pGameObject->GetName());
      break;
    }
    case GameObjectProp::GlobalKey:
    {
      Sqrat::PushVar<const char*>(vm, pGameObject->GetGlobalKey());
      break;
    }
    default:
    {
      EZ_ASSERT_NOT_IMPLEMENTED;
      sq_pushnull(vm);
      break;
    }
  }

  return 1;
}

// ez.GameObject.SetTeamID(integer, integer): void
SQInteger ezspGameObjectSetTeamID(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, -2); pGameObject != nullptr)
  {
    SQInteger id;
    sq_getinteger(vm, -1, &id);

    pGameObject->SetTeamID(static_cast<ezUInt16>(id));
  }

  return 0;
}

// ez.GameObject.GetTeamID(integer): integer
SQInteger ezspGameObjectGetTeamID(HSQUIRRELVM vm)
{
  const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  sq_pushinteger(vm, pGameObject->GetTeamID());
  return 1;
}

// ez.GameObject.SetTags(integer, string...): void
// ez.GameObject.AddTags(integer, string...): void
// ez.GameObject.RemoveTags(integer, string...): void
template <int overload>
SQInteger ezspGameObjectChangeTags(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, 2); pGameObject != nullptr)
  {
    if (overload == 1)
      pGameObject->SetTags(ezTagSet());

    for (SQInteger i = 3, l = sq_gettop(vm); i <= l; ++i)
    {
      const SQChar* szParam;
      sq_getstring(vm, i, &szParam);

      switch (overload)
      {
        case 1:
        case 2:
        {
          pGameObject->SetTag(ezTagRegistry::GetGlobalRegistry().RegisterTag(szParam));
          break;
        }
        case 3:
        {
          pGameObject->RemoveTag(ezTagRegistry::GetGlobalRegistry().RegisterTag(szParam));
          break;
        }
        default:
        {
          EZ_ASSERT_NOT_IMPLEMENTED;
          break;
        }
      }
    }
  }

  return 0;
}

// ez.GameObject.HasAnyTags(integer, string...): bool
// ez.GameObject.HasAllTags(integer, string...): bool
template <int overload>
SQInteger ezspGameObjectCheckTags(HSQUIRRELVM vm)
{
  const ezGameObject* pGameObject = GetGameObjectFromVM(vm, 2);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  for (SQInteger i = 3, l = sq_gettop(vm); i <= l; ++i)
  {
    const SQChar* szParam;
    sq_getstring(vm, i, &szParam);

    const bool bIsSet = pGameObject->GetTags().IsSetByName(szParam);

    switch (overload)
    {
      case 1:
      {
        if (bIsSet)
        {
          sq_pushbool(vm, true);
          return 1;
        }

        break;
      }
      case 2:
      {
        if (!bIsSet)
        {
          sq_pushbool(vm, false);
          return 1;
        }

        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  constexpr bool bValue = overload == 1;
  sq_pushbool(vm, bValue);
  return 1;
}

// ez.GameObject.GetParent(integer): integer
SQInteger ezspGameObjectGetParent(HSQUIRRELVM vm)
{
  const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  const ezGameObject* pParent = pGameObject->GetParent();

  if (pParent != nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  Sqrat::PushVar(vm, pParent->GetHandle());
  return 1;
}

// ez.GameObject.SetParent(integer, integer, bool): void
// ez.GameObject.AddChild(integer, integer, bool): void
// ez.GameObject.DetachChild(integer, integer, bool): void
template <GameObjectProp::Enum property>
SQInteger ezspGameObjectSetGameObjectProp(HSQUIRRELVM vm)
{
  if (ezGameObject* pGameObject = GetGameObjectFromVM(vm, -3); pGameObject != nullptr)
  {
    const Sqrat::Var<ezGameObjectHandle> hGameObject(vm, -2);
    const Sqrat::Var<bool> bPreserve(vm, -1);

    const ezGameObject::TransformPreservation preservation = bPreserve.value ? ezGameObject::TransformPreservation::PreserveGlobal : ezGameObject::TransformPreservation::PreserveLocal;

    switch (property)
    {
      case GameObjectProp::SetParent:
      {
        pGameObject->SetParent(hGameObject.value, preservation);
        break;
      }
      case GameObjectProp::AddChild:
      {
        pGameObject->AddChild(hGameObject.value, preservation);
        break;
      }
      case GameObjectProp::DetachChild:
      {
        pGameObject->DetachChild(hGameObject.value, preservation);
        break;
      }
      default:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
      }
    }
  }

  return 0;
}

// ez.GameObject.GetChildren(integer): integer[]
SQInteger ezspGameObjectGetChildren(HSQUIRRELVM vm)
{
  const ezGameObject* pGameObject = GetGameObjectFromVM(vm, -1);

  if (pGameObject == nullptr)
  {
    sq_pushnull(vm);
    return 1;
  }

  Sqrat::Array handles(vm);
  for (auto it = pGameObject->GetChildren(); it.IsValid(); ++it)
    handles.Append(it->GetHandle());

  Sqrat::PushVar(vm, handles);
  return 1;
}

SQRESULT ezSparkLangModule::ezGameObject(Sqrat::Table& module)
{
  Sqrat::Class<ezTag> TagClass(module.GetVM(), _SC("ezTag"));
  TagClass
    .Ctor()
    .Prop(_SC("TagString"), &ezTag::GetTagString)
    .Prop(_SC("IsValid"), &ezTag::IsValid);

  Sqrat::Class<ezTagSet> TagSetClass(module.GetVM(), _SC("ezTagSet"));
  TagSetClass
    .Ctor()
    .Func(_SC("Set"), &ezTagSet::Set)
    .Func(_SC("Remove"), &ezTagSet::Remove)
    .Func(_SC("IsSet"), &ezTagSet::IsSet)
    .Func(_SC("IsAnySet"), &ezTagSet::IsAnySet)
    .Prop(_SC("NumTagsSet"), &ezTagSet::GetNumTagsSet)
    .Prop(_SC("IsEmpty"), &ezTagSet::IsEmpty)
    .Prop(_SC("Clear"), &ezTagSet::Clear)
    .Func(_SC("SetByName"), &ezTagSet::SetByName)
    .Func(_SC("RemoveByName"), &ezTagSet::RemoveByName)
    .Func(_SC("IsSetByName"), &ezTagSet::IsSetByName);

  Sqrat::Class<ezGameObjectDesc> GameObjectDescClass(module.GetVM(), _SC("ezGameObjectDesc"));
  GameObjectDescClass
    .Var(_SC("ActiveFlag"), &ezGameObjectDesc::m_bActiveFlag)
    .Var(_SC("Dynamic"), &ezGameObjectDesc::m_bDynamic)
    .Var(_SC("TeamID"), &ezGameObjectDesc::m_uiTeamID)
    .Var(_SC("Name"), &ezGameObjectDesc::m_sName)
    .Var(_SC("Parent"), &ezGameObjectDesc::m_hParent)
    .Var(_SC("LocalPosition"), &ezGameObjectDesc::m_LocalPosition)
    .Var(_SC("LocalRotation"), &ezGameObjectDesc::m_LocalRotation)
    .Var(_SC("LocalScaling"), &ezGameObjectDesc::m_LocalScaling)
    .Var(_SC("LocalUniformScaling"), &ezGameObjectDesc::m_LocalUniformScaling)
    .Var(_SC("Tags"), &ezGameObjectDesc::m_Tags)
    .Var(_SC("StableRandomSeed"), &ezGameObjectDesc::m_uiStableRandomSeed);

  Sqrat::Table GameObject(module.GetVM());
  GameObject
    .Bind(_SC("Tag"), TagClass)
    .Bind(_SC("TagSet"), TagSetClass)
    .Bind(_SC("GameObjectDesc"), GameObjectDescClass)

    .SquirrelFunc(_SC("IsValid"), ezspGameObjectIsValid, 2, _SC(".i"))

    .SquirrelFunc(_SC("SetLocalPosition"), ezspGameObjectSetVec3Prop<GameObjectProp::LocalPosition>, 3, _SC(".ix"))
    .SquirrelFunc(_SC("SetGlobalPosition"), ezspGameObjectSetVec3Prop<GameObjectProp::GlobalPosition>, 3, _SC(".ix"))
    .SquirrelFunc(_SC("SetLocalScaling"), ezspGameObjectSetVec3Prop<GameObjectProp::LocalScaling>, 3, _SC(".ix"))
    .SquirrelFunc(_SC("SetGlobalScaling"), ezspGameObjectSetVec3Prop<GameObjectProp::GlobalScaling>, 3, _SC(".ix"))
    .SquirrelFunc(_SC("SetVelocity"), ezspGameObjectSetVec3Prop<GameObjectProp::Velocity>, 3, _SC(".ix"))

    .SquirrelFunc(_SC("GetLocalPosition"), ezspGameObjectGetVec3Prop<GameObjectProp::LocalPosition>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalPosition"), ezspGameObjectGetVec3Prop<GameObjectProp::GlobalPosition>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetLocalScaling"), ezspGameObjectGetVec3Prop<GameObjectProp::LocalScaling>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalScaling"), ezspGameObjectGetVec3Prop<GameObjectProp::GlobalScaling>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalDirUp"), ezspGameObjectGetVec3Prop<GameObjectProp::GlobalDirUp>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalDirRight"), ezspGameObjectGetVec3Prop<GameObjectProp::GlobalDirRight>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalDirForwards"), ezspGameObjectGetVec3Prop<GameObjectProp::GlobalDirForwards>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetVelocity"), ezspGameObjectGetVec3Prop<GameObjectProp::Velocity>, 2, _SC(".i"))

    .SquirrelFunc(_SC("SetLocalUniformScaling"), ezspGameObjectSetFloatProp<GameObjectProp::LocalUniformScaling>, 3, _SC(".if"))

    .SquirrelFunc(_SC("GetLocalUniformScaling"), ezspGameObjectGetFloatProp<GameObjectProp::LocalUniformScaling>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetChildCount"), ezspGameObjectGetFloatProp<GameObjectProp::ChildCount>, 2, _SC(".i"))

    .SquirrelFunc(_SC("SetLocalRotation"), ezspGameObjectSetQuatProp<GameObjectProp::LocalRotation>, 3, _SC(".ix"))
    .SquirrelFunc(_SC("SetGlobalRotation"), ezspGameObjectSetQuatProp<GameObjectProp::GlobalRotation>, 3, _SC(".ix"))

    .SquirrelFunc(_SC("GetLocalRotation"), ezspGameObjectGetQuatProp<GameObjectProp::LocalRotation>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalRotation"), ezspGameObjectGetQuatProp<GameObjectProp::GlobalRotation>, 2, _SC(".i"))

    .SquirrelFunc(_SC("SetActiveFlag"), ezspGameObjectSetBoolProp<GameObjectProp::ActiveFlag>, 3, _SC(".ib"))

    .SquirrelFunc(_SC("GetActiveFlag"), ezspGameObjectGetBoolProp<GameObjectProp::ActiveFlag>, 2, _SC(".i"))
    .SquirrelFunc(_SC("IsActive"), ezspGameObjectGetBoolProp<GameObjectProp::Active>, 2, _SC(".i"))
    .SquirrelFunc(_SC("IsDynamic"), ezspGameObjectGetBoolProp<GameObjectProp::Dynamic>, 2, _SC(".i"))
    .SquirrelFunc(_SC("IsStatic"), ezspGameObjectGetBoolProp<GameObjectProp::Static>, 2, _SC(".i"))

    .SquirrelFunc(_SC("FindChildByName"), ezspGameObjectFindChildByName, 4, _SC(".isb"))
    .SquirrelFunc(_SC("FindChildByPath"), ezspGameObjectFindChildByPath, 3, _SC(".is"))

    .SquirrelFunc(_SC("TryGetComponentOfBaseType"), ezspGameObjectTryGetComponentOfBaseType, 3, _SC(".ii"))
    .SquirrelFunc(_SC("TryGetComponentsOfBaseType"), ezspGameObjectTryGetComponentsOfBaseType, 3, _SC(".ii"))

    .SquirrelFunc(_SC("SearchForChildByNameSequence"), ezspGameObjectSearchForChildByNameSequence, 4, _SC(".iss|n"))
    .SquirrelFunc(_SC("SearchForChildrenByNameSequence"), ezspGameObjectSearchForChildrenByNameSequence, 4, _SC(".iss|n"))

    .SquirrelFunc(_SC("SendMessage"), ezspGameObjectSendMessage<1>, 4, _SC(".ixb"))
    .SquirrelFunc(_SC("PostMessage"), ezspGameObjectSendMessage<2>, 5, _SC(".ixbf"))

    .SquirrelFunc(_SC("SendEventMessage"), ezspGameObjectSendEventMessage<1>, 4, _SC(".ixi"))
    .SquirrelFunc(_SC("PostEventMessage"), ezspGameObjectSendEventMessage<2>, 5, _SC(".ixif"))

    .SquirrelFunc(_SC("SetName"), ezspGameObjectSetStringProp<GameObjectProp::Name>, 3, _SC(".is"))
    .SquirrelFunc(_SC("SetGlobalKey"), ezspGameObjectSetStringProp<GameObjectProp::GlobalKey>, 3, _SC(".is"))

    .SquirrelFunc(_SC("GetName"), ezspGameObjectGetStringProp<GameObjectProp::Name>, 2, _SC(".i"))
    .SquirrelFunc(_SC("GetGlobalKey"), ezspGameObjectGetStringProp<GameObjectProp::GlobalKey>, 2, _SC(".i"))

    .SquirrelFunc(_SC("SetTeamID"), ezspGameObjectSetTeamID, 3, _SC(".ii"))
    .SquirrelFunc(_SC("GetTeamID"), ezspGameObjectGetTeamID, 2, _SC(".i"))

    .SquirrelFunc(_SC("SetTags"), ezspGameObjectChangeTags<1>, -3, _SC(".is"))
    .SquirrelFunc(_SC("AddTags"), ezspGameObjectChangeTags<2>, -3, _SC(".is"))
    .SquirrelFunc(_SC("RemoveTags"), ezspGameObjectChangeTags<3>, -3, _SC(".is"))

    .SquirrelFunc(_SC("HasAnyTags"), ezspGameObjectCheckTags<1>, -3, _SC(".is"))
    .SquirrelFunc(_SC("HasAllTags"), ezspGameObjectCheckTags<2>, -3, _SC(".is"))

    .SquirrelFunc(_SC("GetParent"), ezspGameObjectGetParent, 2, _SC(".i"))

    .SquirrelFunc(_SC("SetParent"), ezspGameObjectSetGameObjectProp<GameObjectProp::SetParent>, 4, _SC(".iib"))
    .SquirrelFunc(_SC("AddChild"), ezspGameObjectSetGameObjectProp<GameObjectProp::AddChild>, 4, _SC(".iib"))
    .SquirrelFunc(_SC("DetachChild"), ezspGameObjectSetGameObjectProp<GameObjectProp::DetachChild>, 4, _SC(".iib"))

    .SquirrelFunc(_SC("GetChildren"), ezspGameObjectGetChildren, 2, _SC(".i"));

  module.Bind(_SC("GameObject"), GameObject);

  return SQ_OK;
}
