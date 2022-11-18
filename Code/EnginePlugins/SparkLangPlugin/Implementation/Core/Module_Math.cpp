#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Components/SparkLangScriptComponent.h>
#include <SparkLangPlugin/Core/ScriptContext.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>

namespace SparkOperator
{
  enum Enum
  {
    Add,
    Sub,
    Mul,
    Div,
    Unm,
    Mod,
    Cmp
  };
}

// ez.Math.Vec2 + ez.Math.Vec2
// ez.Math.Vec2 - ez.Math.Vec2
// ez.Math.Vec2 * ez.Math.Vec2
// ez.Math.Vec2 / ez.Math.Vec2
// ez.Math.Vec2 % ez.Math.Vec2
// ez.Math.Vec2 <=> ez.Math.Vec2
// ez.Math.Vec3 + ez.Math.Vec3
// ez.Math.Vec3 - ez.Math.Vec3
// ez.Math.Vec3 * ez.Math.Vec3
// ez.Math.Vec3 / ez.Math.Vec3
// ez.Math.Vec3 % ez.Math.Vec3
// ez.Math.Vec3 <=> ez.Math.Vec3
// ez.Math.Vec4 + ez.Math.Vec4
// ez.Math.Vec4 - ez.Math.Vec4
// ez.Math.Vec4 * ez.Math.Vec4
// ez.Math.Vec4 / ez.Math.Vec4
// ez.Math.Vec4 % ez.Math.Vec4
// ez.Math.Vec4 <=> ez.Math.Vec4
template <class Vec, SparkOperator::Enum op>
SQInteger ezspMathVecBinaryOperator(HSQUIRRELVM vm)
{
  const auto& self = Sqrat::Var<const Vec&>(vm, -2);
  const auto& other = Sqrat::Var<const Vec&>(vm, -1);

  switch (op)
  {
    case SparkOperator::Add:
      Sqrat::PushVar(vm, self.value + other.value);
      break;
    case SparkOperator::Sub:
      Sqrat::PushVar(vm, self.value - other.value);
      break;
    case SparkOperator::Mul:
      Sqrat::PushVar(vm, self.value.CompMul(other.value));
      break;
    case SparkOperator::Div:
      Sqrat::PushVar(vm, self.value.CompDiv(other.value));
      break;
    case SparkOperator::Mod:
      sq_throwerror(vm, "Cannot use the modulo operator between ezVec3 objects");
      break;
    case SparkOperator::Cmp:
    {
      const float selfLength = self.value.GetLengthSquared();
      const float otherLength = other.value.GetLengthSquared();
      if (selfLength > otherLength)
        sq_pushinteger(vm, 1);
      else if (selfLength < otherLength)
        sq_pushinteger(vm, -1);
      else
        sq_pushinteger(vm, 0);
      break;
    }
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
  }

  return 1;
}

// -ez.Math.Vec2
// -ez.Math.Vec3
// -ez.Math.Vec4
// -ez.Math.Quat
template <class Vec, SparkOperator::Enum op>
SQInteger ezspMathVecUnaryOperator(HSQUIRRELVM vm)
{
  const auto& self = Sqrat::Var<const Vec&>(vm, -1);

  switch (op)
  {
    case SparkOperator::Unm:
      Sqrat::PushVar(vm, -self.value);
      break;
    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
  }

  return 1;
}

SQRESULT ezSparkLangModule::ezMath(Sqrat::Table& module)
{
  Sqrat::Table Math(module.GetVM());

  Sqrat::Class<ezVec2> Vec2Class(module.GetVM(), _SC("ezVec2"));
  Sqrat::Class<ezVec3> Vec3Class(module.GetVM(), _SC("ezVec3"));
  Sqrat::Class<ezVec4> Vec4Class(module.GetVM(), _SC("ezVec4"));
  Sqrat::Class<ezQuat> QuatClass(module.GetVM(), _SC("ezQuat"));

  Vec2Class
    .Ctor()
    .Ctor<float>()
    .Ctor<float, float>()
    .StaticFunc(_SC("ZeroVector"), &ezVec2::ZeroVector)
    .Func(_SC("Set2"), static_cast<void (ezVec2::*)(float, float)>(&ezVec2::Set))
    .Func(_SC("Set1"), static_cast<void (ezVec2::*)(float)>(&ezVec2::Set))
    .Func(_SC("SetZero"), &ezVec2::SetZero)
    .Prop(_SC("Length"), &ezVec2::GetLength)
    .Func(_SC("SetLength"), &ezVec2::SetLength)
    .Prop(_SC("LengthSquared"), &ezVec2::GetLengthSquared)
    .Prop(_SC("LengthAndNormalize"), &ezVec2::GetLengthAndNormalize)
    .Prop(_SC("Normalized"), &ezVec2::GetNormalized)
    .Func(_SC("Normalize"), &ezVec2::Normalize)
    .Prop(_SC("IsZero"), static_cast<bool (ezVec2::*)() const>(&ezVec2::IsZero))
    .Prop(_SC("IsNormalized"), &ezVec2::IsNormalized)
    .Prop(_SC("IsNaN"), &ezVec2::IsNaN)
    .Prop(_SC("IsValid"), &ezVec2::IsValid)
    .Func(_SC("Abs"), &ezVec2::Abs)
    .Func(_SC("GetAsVec4"), &ezVec2::GetAsVec4)
    .Func(_SC("GetAsVec3"), &ezVec2::GetAsVec3)
    .Func(_SC("Dot"), &ezVec2::Dot)
    .SquirrelFunc(_SC("_add"), ezspMathVecBinaryOperator<ezVec2, SparkOperator::Add>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_sub"), ezspMathVecBinaryOperator<ezVec2, SparkOperator::Sub>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_mul"), ezspMathVecBinaryOperator<ezVec2, SparkOperator::Mul>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_div"), ezspMathVecBinaryOperator<ezVec2, SparkOperator::Div>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_cmp"), ezspMathVecBinaryOperator<ezVec2, SparkOperator::Cmp>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_mod"), ezspMathVecBinaryOperator<ezVec2, SparkOperator::Mod>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_unm"), ezspMathVecUnaryOperator<ezVec2, SparkOperator::Unm>, 1, _SC("."));

  Vec3Class
    .Ctor()
    .Ctor<float>()
    .Ctor<float, float, float>()
    .StaticFunc(_SC("ZeroVector"), &ezVec3::ZeroVector)
    .StaticFunc(_SC("OneVector"), &ezVec3::OneVector)
    .StaticFunc(_SC("UnitXAxis"), &ezVec3::UnitXAxis)
    .StaticFunc(_SC("UnitYAxis"), &ezVec3::UnitYAxis)
    .StaticFunc(_SC("UnitZAxis"), &ezVec3::UnitZAxis)
    .Func(_SC("Set3"), static_cast<void (ezVec3::*)(float, float, float)>(&ezVec3::Set))
    .Func(_SC("Set1"), static_cast<void (ezVec3::*)(float)>(&ezVec3::Set))
    .Func(_SC("SetZero"), &ezVec3::SetZero)
    .Prop(_SC("Length"), &ezVec3::GetLength)
    .Func(_SC("SetLength"), &ezVec3::SetLength)
    .Prop(_SC("LengthSquared"), &ezVec3::GetLengthSquared)
    .Prop(_SC("LengthAndNormalize"), &ezVec3::GetLengthAndNormalize)
    .Prop(_SC("Normalized"), &ezVec3::GetNormalized)
    .Func(_SC("Normalize"), &ezVec3::Normalize)
    .Prop(_SC("IsZero"), static_cast<bool (ezVec3::*)() const>(&ezVec3::IsZero))
    .Prop(_SC("IsNormalized"), &ezVec3::IsNormalized)
    .Prop(_SC("IsNaN"), &ezVec3::IsNaN)
    .Prop(_SC("IsValid"), &ezVec3::IsValid)
    .Func(_SC("Abs"), &ezVec3::Abs)
    .Func(_SC("GetAsVec2"), &ezVec3::GetAsVec2)
    .Func(_SC("GetAsVec3"), &ezVec3::GetAsVec4)
    .Func(_SC("Dot"), &ezVec3::Dot)
    .Func(_SC("CrossRH"), &ezVec3::CrossRH)
    .SquirrelFunc(_SC("_add"), ezspMathVecBinaryOperator<ezVec3, SparkOperator::Add>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_sub"), ezspMathVecBinaryOperator<ezVec3, SparkOperator::Sub>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_mul"), ezspMathVecBinaryOperator<ezVec3, SparkOperator::Mul>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_div"), ezspMathVecBinaryOperator<ezVec3, SparkOperator::Div>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_cmp"), ezspMathVecBinaryOperator<ezVec3, SparkOperator::Cmp>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_mod"), ezspMathVecBinaryOperator<ezVec3, SparkOperator::Mod>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_unm"), ezspMathVecUnaryOperator<ezVec3, SparkOperator::Unm>, 1, _SC("."));

  Vec4Class
    .Ctor()
    .Ctor<float>()
    .Ctor<float, float, float, float>()
    .StaticFunc(_SC("ZeroVector"), &ezVec4::ZeroVector)
    .StaticFunc(_SC("OriginPoint"), &ezVec4::OriginPoint)
    .StaticFunc(_SC("OneVector"), &ezVec4::OneVector)
    .StaticFunc(_SC("UnitXAxis"), &ezVec4::UnitXAxis)
    .StaticFunc(_SC("UnitYAxis"), &ezVec4::UnitYAxis)
    .StaticFunc(_SC("UnitZAxis"), &ezVec4::UnitZAxis)
    .Func(_SC("Set4"), static_cast<void (ezVec4::*)(float, float, float, float)>(&ezVec4::Set))
    .Func(_SC("Set1"), static_cast<void (ezVec4::*)(float)>(&ezVec4::Set))
    .Func(_SC("SetZero"), &ezVec4::SetZero)
    .Prop(_SC("Length"), &ezVec4::GetLength)
    .Prop(_SC("LengthSquared"), &ezVec4::GetLengthSquared)
    .Prop(_SC("LengthAndNormalize"), &ezVec4::GetLengthAndNormalize)
    .Prop(_SC("Normalized"), &ezVec4::GetNormalized)
    .Func(_SC("Normalize"), &ezVec4::Normalize)
    .Prop(_SC("IsZero"), static_cast<bool (ezVec4::*)() const>(&ezVec4::IsZero))
    .Prop(_SC("IsNormalized"), &ezVec4::IsNormalized)
    .Prop(_SC("IsNaN"), &ezVec4::IsNaN)
    .Prop(_SC("IsValid"), &ezVec4::IsValid)
    .Func(_SC("Abs"), &ezVec4::Abs)
    .Func(_SC("GetAsVec2"), &ezVec4::GetAsVec2)
    .Func(_SC("GetAsVec3"), &ezVec4::GetAsVec3)
    .Func(_SC("Dot"), &ezVec4::Dot)
    .SquirrelFunc(_SC("_add"), ezspMathVecBinaryOperator<ezVec4, SparkOperator::Add>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_sub"), ezspMathVecBinaryOperator<ezVec4, SparkOperator::Sub>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_mul"), ezspMathVecBinaryOperator<ezVec4, SparkOperator::Mul>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_div"), ezspMathVecBinaryOperator<ezVec4, SparkOperator::Div>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_cmp"), ezspMathVecBinaryOperator<ezVec4, SparkOperator::Cmp>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_mod"), ezspMathVecBinaryOperator<ezVec4, SparkOperator::Mod>, 2, _SC(".x"))
    .SquirrelFunc(_SC("_unm"), ezspMathVecUnaryOperator<ezVec3, SparkOperator::Unm>, 1, _SC("."));

  QuatClass
    .Ctor()
    .Ctor<float, float, float, float>()
    .StaticFunc(_SC("IdentityQuaternion"), &ezQuat::IdentityQuaternion)
    .Func(_SC("SetIdentity"), &ezQuat::SetIdentity)
    .Func(_SC("SetElements"), &ezQuat::SetElements)
    .Func(_SC("SetShortestRotation"), &ezQuat::SetShortestRotation)
    .Func(_SC("SetSlerp"), &ezQuat::SetSlerp)
    .Func(_SC("Normalize"), &ezQuat::Normalize)
    .Prop(_SC("IsNaN"), &ezQuat::IsNaN)
    .Prop(_SC("IsValid"), &ezQuat::IsValid)
    .Func(_SC("IsEqualRotation"), &ezQuat::IsEqualRotation)
    .Func(_SC("Dot"), &ezQuat::Dot)
    .SquirrelFunc(_SC("_unm"), ezspMathVecUnaryOperator<ezQuat, SparkOperator::Unm>, 1, _SC("."));

  Math
    .Bind(_SC("Vec2"), Vec2Class)
    .Bind(_SC("Vec3"), Vec3Class)
    .Bind(_SC("Vec4"), Vec4Class)
    .Bind(_SC("Quat"), QuatClass);

  module.Bind(_SC("Math"), Math);

  return SQ_OK;
}
