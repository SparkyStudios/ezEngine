#pragma once

#include <RHI/RHIDLL.h>

#include <Foundation/Types/Delegate.h>

#include <RHI/Core.h>
#include <RHI/Device.h>

struct spRHIImplementationDescription
{
  /// \brief The graphics API provided by the implementation.
  ezEnum<RHI::spGraphicsApi> m_API;

  /// \brief The supported shader model of the implementation.
  ezString m_sShaderModel;

  /// \brief The shader compiler library used by the implementation.
  ezString m_sShaderCompiler;
};

struct SP_RHI_DLL spRHIImplementationFactory
{
  using Factory = ezDelegate<ezInternal::NewInstance<RHI::spDevice>(ezAllocatorBase*, const RHI::spDeviceDescription&)>;

  static ezInternal::NewInstance<RHI::spDevice> CreateDevice(ezStringView szImplementationName, ezAllocatorBase* pAllocator, const RHI::spDeviceDescription& description);

  static void RegisterImplementation(ezStringView szName, const Factory& func, const spRHIImplementationDescription& description);

  static void UnregisterImplementation(ezStringView szName);
};
