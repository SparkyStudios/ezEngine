#pragma once

#include <SparkLangPlugin/Core/Module.h>

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
    static void push(HSQUIRRELVM vm, ezComponentHandle value)
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
  struct Var<const ezComponentHandle&>
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

    static const SQChar* getVarTypeName() { return _SC("ezComponentHandle const ref"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return sq_gettype(vm, idx) == OT_INTEGER;
    }
  };
} // namespace Sqrat
