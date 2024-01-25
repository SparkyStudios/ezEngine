#include <RHID3D11/RHID3D11PCH.h>

#include <Foundation/Configuration/SubSystem.h>

#include <RHI/Factory.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/ResourceFactory.h>
#include <RHID3D11/ResourceManager.h>

using namespace RHI;

ezInternal::NewInstance<spDeviceD3D11> CreateRHID3D11Device(ezAllocatorBase* pAllocator, const spDeviceDescription& description)
{
  return EZ_NEW(pAllocator, spDeviceD3D11, ezDefaultAllocatorWrapper::GetAllocator(), static_cast<const spDeviceDescriptionD3D11&>(description));
}

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RHID3D11, DeviceFactory)

  ON_CORESYSTEMS_STARTUP
  {
    const spRHIImplementationDescription desc{
      spGraphicsApi::Direct3D11
    };

    spRHIImplementationFactory::RegisterImplementation("D3D11", &CreateRHID3D11Device, desc);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    spRHIImplementationFactory::UnregisterImplementation("D3D11");
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_RHID3D11);
