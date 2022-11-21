#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/ScriptContext.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

struct SpatialSystemFindGameObjectsCallback
{
  Sqrat::Function m_callback;

  ezVisitorExecution::Enum Callback(ezGameObject* pGameObject) const
  {
    if (m_callback.IsNull())
      return ezVisitorExecution::Stop;

    bool result = false;
    m_callback.Evaluate(pGameObject, result);

    return result ? ezVisitorExecution::Continue : ezVisitorExecution::Stop;
  }
};

ezWorld* GetWorldFromVM(HSQUIRRELVM vm)
{
  const ezSparkLangScriptContext* pContext = ezSparkLangScriptContext::FromVM(vm);
  if (pContext == nullptr)
    return nullptr;

  return pContext->GetWorld();
}

// ez.World.DeleteObjectDelayed(integer, bool): void
// ez.World.DeleteObjectNow(integer, bool): void
template <int overload>
SQInteger ezspWorldDeleteObject(HSQUIRRELVM vm)
{
  ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  const Sqrat::Var<ezGameObjectHandle> hObject(vm, -2);
  const Sqrat::Var<bool> bDeleteEmptyParents(vm, -1);

  if constexpr (overload == 1)
    pWorld->DeleteObjectDelayed(hObject.value, bDeleteEmptyParents.value);
  else
    pWorld->DeleteObjectNow(hObject.value, bDeleteEmptyParents.value);

  return 0;
}

// ez.World.CreateObject(ezGameObjectDesc): integer
SQInteger ezspWorldCreateObject(HSQUIRRELVM vm)
{
  ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  const Sqrat::Var<ezGameObjectDesc> desc(vm, -1);

  const ezGameObjectHandle hObject = pWorld->CreateObject(desc.value);

  Sqrat::PushVar(vm, hObject);
  return 1;
}

// ez.World.CreateComponent(integer, integer): integer
SQInteger ezspWorldCreateComponent(HSQUIRRELVM vm)
{
  ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  const Sqrat::Var<ezGameObject*> pGameObject(vm, -2);
  if (pGameObject.value == nullptr)
    return 0;

  const Sqrat::Var<ezUInt32> hash(vm, -1);

  const ezRTTI* pRtti = ezRTTI::FindTypeByNameHash32(hash.value);
  if (pRtti == nullptr)
  {
    sq_throwerror(vm, "Invalid component type");
    return 0;
  }

  auto* pManager = pWorld->GetOrCreateManagerForComponentType(pRtti);

  ezComponent* pComponent;
  pManager->CreateComponent(pGameObject.value, pComponent);

  Sqrat::PushVar(vm, pComponent);
  return 1;
}

// ez.World.DeleteComponent(integer): void
SQInteger ezspWorldDeleteComponent(HSQUIRRELVM vm)
{
  const Sqrat::Var<ezComponent*> pComponent(vm, -1);
  if (pComponent.value == nullptr)
    return 0;

  pComponent.value->GetOwningManager()->DeleteComponent(pComponent.value);
  return 0;
}

// ez.World.TryGetObjectWithGlobalKey(string): integer
SQInteger ezspWorldTryGetObjectWithGlobalKey(HSQUIRRELVM vm)
{
  ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  const Sqrat::Var<const SQChar*> szGlobalKey(vm, -1);

  ezGameObject* pGameObject;
  bool _ = pWorld->TryGetObjectWithGlobalKey(ezTempHashedString(szGlobalKey.value), pGameObject);

  Sqrat::PushVar(vm, pGameObject);
  return 1;
}

// ez.World.FindObjectsInSphere(string, ezVec3, float, function): void
SQInteger ezspWorldFindObjectsInSphere(HSQUIRRELVM vm)
{
  const ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  const Sqrat::Var<const SQChar*> szCategory(vm, -4);
  const Sqrat::Var<ezVec3> vCenter(vm, -3);
  const Sqrat::Var<float> fRadius(vm, -2);
  const Sqrat::Var<Sqrat::Function> callback(vm, -1);

  SpatialSystemFindGameObjectsCallback cb;
  cb.m_callback = callback.value;

  if (const auto& spatialCategory = ezSpatialData::FindCategory(szCategory.value); spatialCategory != ezInvalidSpatialDataCategory)
  {
    ezSpatialSystem::QueryParams queryParams;
    queryParams.m_uiCategoryBitmask = spatialCategory.GetBitmask();

    pWorld->GetSpatialSystem()->FindObjectsInSphere(
      ezBoundingSphere(vCenter.value, fRadius.value),
      queryParams,
      ezMakeDelegate(&SpatialSystemFindGameObjectsCallback::Callback, &cb));
  }

  return 0;
}

// ez.World.FindObjectsInBox(string, ezVec3, ezVec3, function): void
SQInteger ezspWorldFindObjectsInBox(HSQUIRRELVM vm)
{
  const ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  const Sqrat::Var<const SQChar*> szCategory(vm, -4);
  const Sqrat::Var<ezVec3> vMin(vm, -3);
  const Sqrat::Var<ezVec3> vMax(vm, -2);
  const Sqrat::Var<Sqrat::Function> callback(vm, -1);

  SpatialSystemFindGameObjectsCallback cb;
  cb.m_callback = callback.value;

  if (const auto& spatialCategory = ezSpatialData::FindCategory(szCategory.value); spatialCategory != ezInvalidSpatialDataCategory)
  {
    ezSpatialSystem::QueryParams queryParams;
    queryParams.m_uiCategoryBitmask = spatialCategory.GetBitmask();

    pWorld->GetSpatialSystem()->FindObjectsInBox(
      ezBoundingBox(vMin.value, vMax.value),
      queryParams,
      ezMakeDelegate(&SpatialSystemFindGameObjectsCallback::Callback, &cb));
  }

  return 0;
}

// ez.World.GetRandomNumberGenerator(): ezRandom
SQInteger ezspWorldGetRandomNumberGenerator(HSQUIRRELVM vm)
{
  ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  Sqrat::PushVar<ezRandom&>(vm, pWorld->GetRandomNumberGenerator());
  return 1;
}

// ez.World.GetClock(): ezClock
SQInteger ezspWorldGetClock(HSQUIRRELVM vm)
{
  ezWorld* pWorld = GetWorldFromVM(vm);
  if (pWorld == nullptr)
    return 0;

  Sqrat::PushVar<ezClock&>(vm, pWorld->GetClock());
  return 1;
}

SQRESULT ezSparkLangModule::ezWorld(Sqrat::Table& module)
{
  Sqrat::Table World(module.GetVM());
  World
    .SquirrelFunc(_SC("DeleteObjectDelayed"), ezspWorldDeleteObject<1>, 3, _SC(".ib"))
    .SquirrelFunc(_SC("DeleteObjectNow"), ezspWorldDeleteObject<2>, 3, _SC(".ib"))
    .SquirrelFunc(_SC("CreateObject"), ezspWorldCreateObject, 2, _SC(".x"))
    .SquirrelFunc(_SC("CreateComponent"), ezspWorldCreateComponent, 3, _SC(".ii"))
    .SquirrelFunc(_SC("DeleteComponent"), ezspWorldDeleteComponent, 2, _SC(".i"))
    .SquirrelFunc(_SC("TryGetObjectWithGlobalKey"), ezspWorldTryGetObjectWithGlobalKey, 2, _SC(".s"))
    .SquirrelFunc(_SC("FindObjectsInSphere"), ezspWorldFindObjectsInSphere, 5, _SC(".sxfc"))
    .SquirrelFunc(_SC("FindObjectsInBox"), ezspWorldFindObjectsInBox, 5, _SC(".sxxc"))
    .SquirrelFunc(_SC("GetRandomNumberGenerator"), ezspWorldGetRandomNumberGenerator, 1, _SC("."))
    .SquirrelFunc(_SC("GetClock"), ezspWorldGetClock, 1, _SC("."));

  module.Bind(_SC("World"), World);

  return SQ_OK;
}
