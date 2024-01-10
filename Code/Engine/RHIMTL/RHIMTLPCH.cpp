#include <Foundation/Basics/Platform/Common.h>
#include <RHIMTL/RHIMTLPCH.h>

EZ_STATICLINK_LIBRARY(RHIMTL)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Buffer);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_CommandList);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Device);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Fence);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Framebuffer);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_InputLayout);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Pipeline);
  // EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Profiler);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_ResourceFactory);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_ResourceLayout);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_ResourceSet);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Sampler);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Shader);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Swapchain);
  EZ_STATICLINK_REFERENCE(RHIMTL_Implementation_Texture);
  EZ_STATICLINK_REFERENCE(RHIMTL_RHIMTL);
}
