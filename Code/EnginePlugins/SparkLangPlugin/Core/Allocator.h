#pragma once

#include <SparkLangPlugin/SparkLangPluginDLL.h>

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Memory/CommonAllocators.h>

/// \brief An allocator for the SparkLang virtual machine.
class EZ_SPARKLANGPLUGIN_DLL ezSparkLangAllocator : public ezHeapAllocator
{
  EZ_DECLARE_SINGLETON(ezSparkLangAllocator);

public:
  ezSparkLangAllocator();
};

/// \brief EZ allocation context.
struct SQAllocContextT
{
  ezSparkLangAllocator* m_pAllocator = nullptr;
  bool m_bIsOwned = false;
};
