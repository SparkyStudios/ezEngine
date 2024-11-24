// Copyright (c) 2023-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <RPI/RPIPCH.h>

EZ_STATICLINK_LIBRARY(RPI)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(RPI_Implementation_Assets_BlendShape);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Assets_Image);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Assets_Material);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Assets_Mesh);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Assets_RootMaterial);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Assets_Sampler);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Assets_Shader);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Core_RenderContext);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Core_RenderThread);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Pipeline_RenderPipeline);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_Loaders_RootMaterialResourceLoader);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_Loaders_ShaderResourceLoader);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_Loaders_TextureResourceLoader);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_BlendShapeResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_ImageResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_MaterialResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_MeshResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_RootMaterialResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_SamplerResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_ShaderResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_SkeletonResource);
  EZ_STATICLINK_REFERENCE(RPI_Implementation_Resources_Texture2DResource);
}
