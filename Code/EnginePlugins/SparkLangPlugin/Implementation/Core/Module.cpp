#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Module.h>

#include <Core/World/World.h>

ezVariant GetVariantFromVM(HSQUIRRELVM vm, SQInteger index, const ezRTTI* pRtti)
{
  if (pRtti->IsDerivedFrom<ezEnumBase>() || pRtti->IsDerivedFrom<ezBitflagsBase>())
  {
    SQInteger value;
    sq_getinteger(vm, index, &value);
    return value;
  }

  HSQOBJECT value;
  sq_getstackobj(vm, index, &value);

  switch (pRtti->GetVariantType())
  {
    case ezVariantType::Invalid:
    {
      if (ezStringUtils::IsEqual(pRtti->GetTypeName(), "ezVariant"))
      {
        if (sq_isbool(value))
          return sq_objtobool(&value);

        if (sq_isfloat(value))
          return sq_objtofloat(&value);

        if (sq_isinteger(value))
          return sq_objtointeger(&value);

        if (sq_isstring((value)))
          return sq_objtostring(&value);

        return {};
      }

      return {};
    }

    case ezVariantType::Bool:
      return sq_objtobool(&value) != 0;

    case ezVariantType::Int8:
    case ezVariantType::Int16:
    case ezVariantType::Int32:
    case ezVariantType::Int64:
      return sq_objtointeger(&value);

    case ezVariantType::UInt8:
    case ezVariantType::UInt16:
    case ezVariantType::UInt32:
    case ezVariantType::UInt64:
      return static_cast<SQUnsignedInteger>(sq_objtointeger(&value));

    case ezVariantType::Float:
    case ezVariantType::Double:
      return sq_objtofloat(&value);

    case ezVariantType::Color:
    {
      const Sqrat::Var<ezColor> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector2:
    {
      const Sqrat::Var<ezVec2> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector3:
    {
      const Sqrat::Var<ezVec3> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector4:
    {
      const Sqrat::Var<ezVec4> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector2I:
    {
      const Sqrat::Var<ezVec2I32> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector3I:
    {
      const Sqrat::Var<ezVec3I32> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector4I:
    {
      const Sqrat::Var<ezVec4I32> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector2U:
    {
      const Sqrat::Var<ezVec2U32> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector3U:
    {
      const Sqrat::Var<ezVec2U32> var(vm, index);
      return var.value;
    }

    case ezVariantType::Vector4U:
    {
      const Sqrat::Var<ezVec4U32> var(vm, index);
      return var.value;
    }

    case ezVariantType::Quaternion:
    {
      const Sqrat::Var<ezQuat> var(vm, index);
      return var.value;
    }

    case ezVariantType::Matrix3:
    {
      const Sqrat::Var<ezMat3> var(vm, index);
      return var.value;
    }

    case ezVariantType::Matrix4:
    {
      const Sqrat::Var<ezMat4> var(vm, index);
      return var.value;
    }

    case ezVariantType::Transform:
    {
      const Sqrat::Var<ezTransform> var(vm, index);
      return var.value;
    }

    case ezVariantType::String:
      return sq_objtostring(&value);

    case ezVariantType::StringView:
      return ezStringView(sq_objtostring(&value));

    case ezVariantType::DataBuffer:
      break;

    case ezVariantType::Time:
    {
      const Sqrat::Var<ezTime> var(vm, index);
      return var.value;
    }

    case ezVariantType::Uuid:
    {
      const Sqrat::Var<ezUuid> var(vm, index);
      return var.value;
    }

    case ezVariantType::Angle:
    {
      const Sqrat::Var<ezAngle> var(vm, index);
      return var.value;
    }

    case ezVariantType::ColorGamma:
    {
      const Sqrat::Var<ezColor> vec(vm, index);
      return ezColorGammaUB(vec.value);
    }

    default:
    {
      EZ_ASSERT_NOT_IMPLEMENTED;
      return {};
    }
  }

  return {};
}

void PushVariantToVM(HSQUIRRELVM vm, const ezVariant& value)
{
  switch (value.GetType())
  {
    case ezVariant::Type::Angle:
      Sqrat::PushVar(vm, value.Get<ezAngle>());
      break;

    case ezVariant::Type::Time:
      Sqrat::PushVar(vm, value.Get<ezTime>());
      break;

    case ezVariant::Type::Bool:
      sq_pushbool(vm, value.Get<bool>());
      break;

    case ezVariant::Type::Int8:
    case ezVariant::Type::UInt8:
    case ezVariant::Type::Int16:
    case ezVariant::Type::UInt16:
    case ezVariant::Type::Int32:
    case ezVariant::Type::UInt32:
    case ezVariant::Type::Int64:
    case ezVariant::Type::UInt64:
      sq_pushinteger(vm, value.ConvertTo<ezUInt64>());
      break;

    case ezVariant::Type::Float:
    case ezVariant::Type::Double:
      sq_pushfloat(vm, value.ConvertTo<double>());
      break;

    case ezVariant::Type::Color:
    case ezVariant::Type::ColorGamma:
      Sqrat::PushVar(vm, value.ConvertTo<ezColor>());
      break;

    case ezVariant::Type::Vector2:
      Sqrat::PushVar(vm, value.Get<ezVec2>());
      break;

    case ezVariant::Type::Vector3:
      Sqrat::PushVar(vm, value.Get<ezVec3>());
      break;

    case ezVariant::Type::Quaternion:
      Sqrat::PushVar(vm, value.Get<ezQuat>());
      break;

    case ezVariant::Type::Transform:
      Sqrat::PushVar(vm, value.Get<ezTransform>());
      break;

    case ezVariant::Type::String:
      Sqrat::PushVar(vm, value.Get<ezString>().GetData());
      break;

    case ezVariant::Type::StringView:
    {
      ezStringBuilder temp;
      Sqrat::PushVar(vm, value.Get<ezStringView>().GetData(temp));
      break;
    }

    case ezVariant::Type::Vector2I:
    {
      const ezVec2I32 v = value.Get<ezVec2I32>();
      Sqrat::PushVar(vm, ezVec2(static_cast<float>(v.x), static_cast<float>(v.y)));
      break;
    }

    case ezVariant::Type::Vector2U:
    {
      const ezVec2U32 v = value.Get<ezVec2U32>();
      Sqrat::PushVar(vm, ezVec2(static_cast<float>(v.x), static_cast<float>(v.y)));
      break;
    }

    case ezVariant::Type::Vector3I:
    {
      const ezVec3I32 v = value.Get<ezVec3I32>();
      Sqrat::PushVar(vm, ezVec3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z)));
      break;
    }

    case ezVariant::Type::Vector3U:
    {
      const ezVec3U32 v = value.Get<ezVec3U32>();
      Sqrat::PushVar(vm, ezVec3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z)));
      break;
    }

    case ezVariant::Type::Matrix3:
      Sqrat::PushVar(vm, value.Get<ezMat3>());
      break;

    case ezVariant::Type::Matrix4:
      Sqrat::PushVar(vm, value.Get<ezMat4>());
      break;

      // TODO: implement these types
      // case ezVariant::Type::Vector4:
      // case ezVariant::Type::Vector4I:
      // case ezVariant::Type::Vector4U:

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      sq_pushnull(vm);
      break;
  }
}

ezSparkLangModule::ezSparkLangModule() = default;

bool ezSparkLangModule::Register(SqModules& modules)
{
  HSQOBJECT hModule;
  sq_newtable(modules.getVM());
  sq_getstackobj(modules.getVM(), -1, &hModule);

  {
    Sqrat::Table module(hModule, modules.getVM());

    if (SQ_FAILED(ezClock(module)))
      return false;

    if (SQ_FAILED(ezComponent(module)))
      return false;

    if (SQ_FAILED(ezGameObject(module)))
      return false;

    if (SQ_FAILED(ezLog(module)))
      return false;

    if (SQ_FAILED(ezMath(module)))
      return false;

    if (SQ_FAILED(ezRandom(module)))
      return false;

    if (SQ_FAILED(ezWorld(module)))
      return false;
  }

  const bool bDone = modules.addNativeModule("ez", SqModules::SqObjPtr(modules.getVM(), hModule));

  sq_poptop(modules.getVM());
  return bDone;
}
