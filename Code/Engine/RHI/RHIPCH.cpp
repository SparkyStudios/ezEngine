#include <RHI/RHIPCH.h>

EZ_STATICLINK_LIBRARY(RHI)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(RHI_Implementation_Buffer);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_CommandList);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Device);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Framebuffer);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Output);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Pipeline);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Rendering);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Resource);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_ResourceLayout);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_ResourceSet);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Sampler);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Shader);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Swapchain);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Texture);

  EZ_STATICLINK_REFERENCE(RHI_Memory_Implementation_StagingMemoryPool);
}
