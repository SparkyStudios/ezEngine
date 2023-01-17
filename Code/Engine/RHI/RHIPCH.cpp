#include <RHI/RHIPCH.h>

EZ_STATICLINK_LIBRARY(RHI)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(RHI_Implementation_Device);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Framebuffer);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Output);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Rendering);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Resource);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Sampler);
  EZ_STATICLINK_REFERENCE(RHI_Implementation_Shader);

  EZ_STATICLINK_REFERENCE(RHI_Memory_Implementation_StagingMemoryPool);
}
