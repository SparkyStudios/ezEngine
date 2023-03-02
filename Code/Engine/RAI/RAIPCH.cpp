#include <RHID3D11/RHID3D11PCH.h>

EZ_STATICLINK_LIBRARY(RAI)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(RAI_Implementation_Import_MeshImporter);
  EZ_STATICLINK_REFERENCE(RAI_Implementation_Resources_MeshResource);
  EZ_STATICLINK_REFERENCE(RAI_Implementation_Mesh);
}
