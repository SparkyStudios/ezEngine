#pragma once

#include <Core/World/Component.h>

#include <sqrat.h>

ezWorld* GetWorldFromVM(HSQUIRRELVM vm);

ezComponent* GetComponentFromVM(HSQUIRRELVM vm, SQInteger index);

ezGameObject* GetGameObjectFromVM(HSQUIRRELVM vm, SQInteger index);

ezVariant GetVariantFromVM(HSQUIRRELVM vm, SQInteger index, const ezRTTI* pRtti);

void PushVariantToVM(HSQUIRRELVM vm, const ezVariant& value);

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
  struct Var<ezComponent*>
  {
    ezComponent* value; ///< The actual value of get operations

    /// Attempts to get the value off the stack at idx as the given type
    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      value = GetComponentFromVM(vm, idx);
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezComponent* value)
    {
      PushVar(vm, value->GetHandle());
    }

    static const SQChar* getVarTypeName() { return _SC("ezComponent ptr"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return Var<ezComponentHandle>::check_type(vm, idx);
    }
  };

  template <>
  struct Var<const ezComponent*>
  {
    const ezComponent* value; ///< The actual value of get operations

    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      value = GetComponentFromVM(vm, idx);
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezComponent* value)
    {
      PushVar(vm, value->GetHandle());
    }

    static const SQChar* getVarTypeName() { return _SC("ezComponent const ptr"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return Var<ezComponentHandle>::check_type(vm, idx);
    }
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
  struct Var<ezGameObject*>
  {
    ezGameObject* value; ///< The actual value of get operations

    /// Attempts to get the value off the stack at idx as the given type
    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      value = GetGameObjectFromVM(vm, idx);
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezGameObject* value)
    {
      PushVar(vm, value->GetHandle());
    }

    static const SQChar* getVarTypeName() { return _SC("ezGameObject ptr"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return Var<ezGameObjectHandle>::check_type(vm, idx);
    }
  };

  template <>
  struct Var<const ezGameObject*>
  {
    const ezGameObject* value; ///< The actual value of get operations

    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      value = GetGameObjectFromVM(vm, idx);
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezGameObject* value)
    {
      PushVar(vm, value->GetHandle());
    }

    static const SQChar* getVarTypeName() { return _SC("ezGameObject const ptr"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return Var<ezGameObjectHandle>::check_type(vm, idx);
    }
  };

  template <>
  struct Var<ezHashedString>
  {
    ezHashedString value; ///< The actual value of get operations

    /// Attempts to get the value off the stack at idx as the given type
    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      const SQChar* string;
      sq_getstring(vm, idx, &string);

      value.Assign(string);
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezHashedString& value)
    {
      sq_pushstring(vm, value.GetData(), -1);
    }

    static const SQChar* getVarTypeName() { return _SC("ezHashedString"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return sq_gettype(vm, idx) == OT_STRING;
    }
  };

  template <>
  struct Var<ezHashedString&> : Var<ezHashedString>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezHashedString>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezHashedString ref"); }
  };

  template <>
  struct Var<const ezHashedString&> : Var<ezHashedString>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezHashedString>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezHashedString const ref"); }
  };

  template <>
  struct Var<ezTempHashedString>
  {
    ezTempHashedString value; ///< The actual value of get operations

    /// Attempts to get the value off the stack at idx as the given type
    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      HSQOBJECT obj;
      sq_getstackobj(vm, idx, &obj);

      if (sq_isstring(obj))
        value = ezTempHashedString(sq_objtostring(&obj));
      else if (sq_isinteger(obj))
        value = ezTempHashedString(static_cast<ezUInt64>(sq_objtointeger(&obj)));
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezTempHashedString& value)
    {
      sq_pushinteger(vm, static_cast<SQInteger>(value.GetHash()));
    }

    static const SQChar* getVarTypeName() { return _SC("ezTempHashedString"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return sq_gettype(vm, idx) == OT_STRING;
    }
  };

  template <>
  struct Var<ezTempHashedString&> : Var<ezTempHashedString>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezTempHashedString>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezTempHashedString ref"); }
  };

  template <>
  struct Var<const ezTempHashedString&> : Var<ezTempHashedString>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezTempHashedString>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezTempHashedString const ref"); }
  };

  template <>
  struct Var<ezString>
  {
    ezString value; ///< The actual value of get operations

    /// Attempts to get the value off the stack at idx as the given type
    Var(HSQUIRRELVM vm, SQInteger idx)
    {
      const SQChar* string;
      sq_getstring(vm, idx, &string);

      value = string;
    }

    /// Called by Sqrat::PushVarR to put a class object on the stack
    static void push(HSQUIRRELVM vm, const ezString& value)
    {
      sq_pushstring(vm, value.GetData(), -1);
    }

    static const SQChar* getVarTypeName() { return _SC("ezString"); }

    static bool check_type(HSQUIRRELVM vm, SQInteger idx)
    {
      return sq_gettype(vm, idx) == OT_STRING;
    }
  };

  template <>
  struct Var<ezString&> : Var<ezString>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezString>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezString ref"); }
  };

  template <>
  struct Var<const ezString&> : Var<ezString>
  {
    Var(HSQUIRRELVM vm, SQInteger idx)
      : Var<ezString>(vm, idx)
    {
    }

    static const SQChar* getVarTypeName() { return _SC("ezString const ref"); }
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

  template <>
  struct InstanceToString<ezColor>
  {
    static SQInteger Format(HSQUIRRELVM vm)
    {
      const auto& vec3 = Var<ezColor>(vm, 1);

      ezStringBuilder sb;
      sb.Format("{}", vec3.value);

      sq_pushstring(vm, sb.GetData(), -1);
      return 1;
    }
  };

  template <>
  struct InstanceToString<ezTag>
  {
    static SQInteger Format(HSQUIRRELVM vm)
    {
      const Var<ezTag> tag(vm, 1);

      PushVar(vm, tag.value.GetTagString());
      return 1;
    }
  };
} // namespace Sqrat
