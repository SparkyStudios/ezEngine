#pragma once

#include <SparkLangPlugin/SparkLangPluginDLL.h>

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>

#include <sqrat.h>

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptProperty : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSparkLangScriptProperty, ezReflectedClass);

public:
  ezSparkLangScriptProperty(ezStringView sName = ezStringView());

  EZ_NODISCARD ezStringView GetName() const
  {
    return m_sName;
  }

  EZ_NODISCARD const ezHashedString& GetNameHashed() const
  {
    return m_sName;
  }

private:
  ezHashedString m_sName;
  Sqrat::Object m_PropertyKey;
};

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptProperty_String : public ezSparkLangScriptProperty
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSparkLangScriptProperty_String, ezSparkLangScriptProperty);

public:
  ezSparkLangScriptProperty_String(ezStringView sName = ezStringView());
  ~ezSparkLangScriptProperty_String() override;

  void SetValue(const SQChar* szValue);

private:
  ezString m_sValue;
};

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptProperty_Integer : public ezSparkLangScriptProperty
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSparkLangScriptProperty_Integer, ezSparkLangScriptProperty);

public:
  ezSparkLangScriptProperty_Integer(ezStringView sName = ezStringView());
  ~ezSparkLangScriptProperty_Integer() override;

  void SetValue(SQInteger iValue);

private:
  SQInteger m_iValue;
};

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptProperty_Float : public ezSparkLangScriptProperty
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSparkLangScriptProperty_Float, ezSparkLangScriptProperty);

public:
  ezSparkLangScriptProperty_Float(ezStringView sName = ezStringView());
  ~ezSparkLangScriptProperty_Float() override;

  void SetValue(SQFloat fValue);

private:
  SQFloat m_fValue;
};

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptProperty_Pointer : public ezSparkLangScriptProperty
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSparkLangScriptProperty_Pointer, ezSparkLangScriptProperty);

public:
  ezSparkLangScriptProperty_Pointer(ezStringView sName = ezStringView());
  ~ezSparkLangScriptProperty_Pointer() override;

  void SetValue(SQUserPointer pValue);

private:
  SQUserPointer m_pValue;
};

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptProperty_Bool : public ezSparkLangScriptProperty
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSparkLangScriptProperty_Bool, ezSparkLangScriptProperty);

public:
  ezSparkLangScriptProperty_Bool(ezStringView sName = ezStringView());
  ~ezSparkLangScriptProperty_Bool() override;

  void SetValue(SQBool bValue);

private:
  SQBool m_bValue;
};
