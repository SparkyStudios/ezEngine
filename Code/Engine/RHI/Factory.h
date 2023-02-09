#pragma once

#include <RHI/RHIDLL.h>

#include <Foundation/Types/Delegate.h>

#include <RHI/Core.h>
#include <RHI/Device.h>

struct spRHIImplementationDescription
{
  /// \brief The graphics API provided by the implementation.
  ezEnum<spGraphicsApi> m_API;

  /// \brief The supported shader model of the implementation.
  ezString m_sShaderModel;

  /// \brief The shader compiler library used by the implementation.
  ezString m_sShaderCompiler;
};

struct SP_RHI_DLL spRHIImplementationFactory
{
  using Factory = ezDelegate<ezInternal::NewInstance<spDevice>(ezAllocatorBase*, const spDeviceDescription&)>;

  static ezInternal::NewInstance<spDevice> CreateDevice(const char* szImplementationName, ezAllocatorBase* pAllocator, const spDeviceDescription& description);

  static void RegisterImplementation(const char* szName, const Factory& func, const spRHIImplementationDescription& description);

  static void UnregisterImplementation(const char* szName);
};
