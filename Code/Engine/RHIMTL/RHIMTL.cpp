#include <RHIMTL/RHIMTLPCH.h>

#include <Foundation/Configuration/SubSystem.h>

#include <RHI/Factory.h>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <RHIMTL/Device.h>
#include <RHIMTL/ResourceFactory.h>

using namespace RHI;

ezInternal::NewInstance<spDeviceMTL> CreateRHIMTLDevice(ezAllocatorBase* pAllocator, const spDeviceDescription& description)
{
  return EZ_NEW(pAllocator, spDeviceMTL, ezDefaultAllocatorWrapper::GetAllocator(), description);
}

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RHIMTL, DeviceFactory)

  ON_CORESYSTEMS_STARTUP
  {
    const spRHIImplementationDescription desc{
      spGraphicsApi::Metal,
      "",
      "ezShaderCompilerMTL"
    };

    spRHIImplementationFactory::RegisterImplementation("MTL", &CreateRHIMTLDevice, desc);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    spRHIImplementationFactory::UnregisterImplementation("MTL");
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_RHIMTL);
