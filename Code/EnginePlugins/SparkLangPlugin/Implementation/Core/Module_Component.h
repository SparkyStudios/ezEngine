#pragma once

#include <Core/World/Component.h>

#include <sqrat.h>

namespace Sqrat
{
  template <>
  struct Var<ezComponentHandle>
  {
    ezComponentHandle value; ///< The actual value of get operations

    /// Attempts to get the value off the stack at idx as the given type
    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      SQInteger id = 0;
      sq_getinteger(vm, idx, &id);

      value = ezComponentHandle(ezComponentId(id));
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezComponentHandle& value)
    {
      sq_pushinteger(vm, static_cast<SQInteger>(value.GetInternalID().m_Data));
    }

    static const SQChar* getVarTypeName() { return _SC("ezComponentHandle"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return sq_gettype(vm, idx) == OT_INTEGER;
    }
  };

  template <>
  struct Var<ezComponentHandle&> : Var<ezComponentHandle>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezComponentHandle>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezComponentHandle ref"); }
  };

  template <>
  struct Var<const ezComponentHandle&> : Var<ezComponentHandle>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezComponentHandle>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezComponentHandle const ref"); }
  };

  template <>
  struct Var<ezGameObjectHandle>
  {
    ezGameObjectHandle value; ///< The actual value of get operations

    /// Attempts to get the value off the stack at idx as the given type
    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      SQInteger id = 0;
      sq_getinteger(vm, idx, &id);

      value = ezGameObjectHandle(ezGameObjectId(id));
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezGameObjectHandle& value)
    {
      sq_pushinteger(vm, static_cast<SQInteger>(value.GetInternalID().m_Data));
    }

    static const SQChar* getVarTypeName() { return _SC("ezGameObjectHandle"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return sq_gettype(vm, idx) == OT_INTEGER;
    }
  };

  template <>
  struct Var<ezGameObjectHandle&> : Var<ezGameObjectHandle>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezGameObjectHandle>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezGameObjectHandle ref"); }
  };

  template <>
  struct Var<const ezGameObjectHandle&> : Var<ezGameObjectHandle>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezGameObjectHandle>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezGameObjectHandle const ref"); }
  };

  template <>
  struct InstanceToString<ezTime>
  {
    static SQInteger Format(HSQUIRRELVM vm)
    {
      const auto& vec3 = Var<ezTime>(vm, 1);

      ezStringBuilder sb;
      sb.Format("{}", vec3.value);

      sq_pushstring(vm, sb.GetData(), -1);
      return 1;
    }
  };

  template <>
  struct InstanceToString<ezVec2>
  {
    static SQInteger Format(HSQUIRRELVM vm)
    {
      const auto& vec3 = Var<ezVec2>(vm, 1);

      ezStringBuilder sb;
      sb.Format("{}", vec3.value);

      sq_pushstring(vm, sb.GetData(), -1);
      return 1;
    }
  };

  template <>
  struct InstanceToString<ezVec3>
  {
    static SQInteger Format(HSQUIRRELVM vm)
    {
      const auto& vec3 = Var<ezVec3>(vm, 1);

      ezStringBuilder sb;
      sb.Format("{}", vec3.value);

      sq_pushstring(vm, sb.GetData(), -1);
      return 1;
    }
  };

  template <>
  struct InstanceToString<ezVec4>
  {
    static SQInteger Format(HSQUIRRELVM vm)
    {
      const auto& vec3 = Var<ezVec4>(vm, 1);

      ezStringBuilder sb;
      sb.Format("{}", vec3.value);

      sq_pushstring(vm, sb.GetData(), -1);
      return 1;
    }
  };

  template <>
  struct InstanceToString<ezQuat>
  {
    static SQInteger Format(HSQUIRRELVM vm)
    {
      const auto& vec3 = Var<ezQuat>(vm, 1);

      ezStringBuilder sb;
      sb.Format("{}", vec3.value);

      sq_pushstring(vm, sb.GetData(), -1);
      return 1;
    }
  };
} // namespace Sqrat

ezComponent* GetComponentFromVM(HSQUIRRELVM vm, SQInteger index);

ezGameObject* GetGameObjectFromVM(HSQUIRRELVM vm, SQInteger index);
