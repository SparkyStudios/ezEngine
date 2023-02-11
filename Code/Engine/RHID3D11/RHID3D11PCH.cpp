#include "Foundation/Basics/Platform/Common.h"
#include <RHID3D11/RHID3D11PCH.h>

EZ_STATICLINK_LIBRARY(RHID3D11)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Buffer);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_CommandList);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Device);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Fence);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Framebuffer);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_InputLayout);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Pipeline);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_ResourceFactory);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_ResourceLayout);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_ResourceManager);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_ResourceSet);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Sampler);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Shader);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Swapchain);
  EZ_STATICLINK_REFERENCE(RHID3D11_Implementation_Texture);
  EZ_STATICLINK_REFERENCE(RHID3D11_RHID3D11);
}
