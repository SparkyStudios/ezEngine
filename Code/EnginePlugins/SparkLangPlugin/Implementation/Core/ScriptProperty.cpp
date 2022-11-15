#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/ScriptProperty.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptProperty, 1, ezRTTINoAllocator)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY_READ_ONLY("Name", GetName),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptProperty_String, 1, ezRTTIDefaultAllocator<ezSparkLangScriptProperty_String>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Value", m_sValue),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptProperty_Bool, 1, ezRTTIDefaultAllocator<ezSparkLangScriptProperty_Bool>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Value", m_bValue),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptProperty_Float, 1, ezRTTIDefaultAllocator<ezSparkLangScriptProperty_Float>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Value", m_fValue),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptProperty_Integer, 1, ezRTTIDefaultAllocator<ezSparkLangScriptProperty_Integer>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Value", m_iValue),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSparkLangScriptProperty_Pointer, 1, ezRTTIDefaultAllocator<ezSparkLangScriptProperty_Pointer>)
{
  // EZ_BEGIN_PROPERTIES
  // {
  //   EZ_MEMBER_PROPERTY("Value", m_pValue),
  // }
  // EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezSparkLangScriptProperty::ezSparkLangScriptProperty(ezStringView sName)
{
  m_sName.Assign(sName);
}

ezSparkLangScriptProperty_String::ezSparkLangScriptProperty_String(ezStringView sName)
  : ezSparkLangScriptProperty(sName)
{
}

ezSparkLangScriptProperty_String::~ezSparkLangScriptProperty_String()
{
}

void ezSparkLangScriptProperty_String::SetValue(const SQChar* szValue)
{
  m_sValue = szValue;
}

ezSparkLangScriptProperty_Integer::ezSparkLangScriptProperty_Integer(ezStringView sName)
  : ezSparkLangScriptProperty(sName)
{
}

ezSparkLangScriptProperty_Integer::~ezSparkLangScriptProperty_Integer()
{
}

void ezSparkLangScriptProperty_Integer::SetValue(SQInteger iValue)
{
  m_iValue = iValue;
}

ezSparkLangScriptProperty_Float::ezSparkLangScriptProperty_Float(ezStringView sName)
  : ezSparkLangScriptProperty(sName)
{
}

ezSparkLangScriptProperty_Float::~ezSparkLangScriptProperty_Float()
{
}

void ezSparkLangScriptProperty_Float::SetValue(SQFloat fValue)
{
  m_fValue = fValue;
}

ezSparkLangScriptProperty_Pointer::ezSparkLangScriptProperty_Pointer(ezStringView sName)
  : ezSparkLangScriptProperty(sName)
{
}

ezSparkLangScriptProperty_Pointer::~ezSparkLangScriptProperty_Pointer()
{
}

void ezSparkLangScriptProperty_Pointer::SetValue(SQUserPointer pValue)
{
  m_pValue = pValue;
}

ezSparkLangScriptProperty_Bool::ezSparkLangScriptProperty_Bool(ezStringView sName)
  : ezSparkLangScriptProperty(sName)
{
}

ezSparkLangScriptProperty_Bool::~ezSparkLangScriptProperty_Bool()
{
}

void ezSparkLangScriptProperty_Bool::SetValue(SQBool bValue)
{
  m_bValue = bValue;
}
