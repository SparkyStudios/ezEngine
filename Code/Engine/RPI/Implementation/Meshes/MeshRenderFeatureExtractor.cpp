// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <RPI/Core/RenderContext.h>
#include <RPI/Core/VisibilityGroup.h>
#include <RPI/Meshes/MeshRenderFeatureExtractor.h>

#include <Core/World/World.h>

namespace RPI
{
  typedef ezRTTIDefaultAllocator<spExtractMeshRenderObjectMessage, RHI::spDeviceAllocatorWrapper> spRTTIExtractMeshRenderObjectMessageAllocator;
  typedef ezRTTIDefaultAllocator<spMeshRenderFeatureExtractor, RHI::spDeviceAllocatorWrapper> spRTTIMeshRenderFeatureExtractorAllocator;

  // clang-format off
  EZ_IMPLEMENT_MESSAGE_TYPE(spExtractMeshRenderObjectMessage);

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spExtractMeshRenderObjectMessage, 1, spRTTIExtractMeshRenderObjectMessageAllocator)
  {
    EZ_BEGIN_ATTRIBUTES
    {
      new ezExcludeFromScript()
    }
    EZ_END_ATTRIBUTES;
  }
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMeshRenderFeatureExtractor, 1, spRTTIMeshRenderFeatureExtractorAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  ezResult spExtractMeshRenderObjectMessage::AddMeshResource(const ezTransform& transform, RAI::spMeshResourceHandle hMeshResource)
  {
    return ezResult(EZ_SUCCESS);
  }

  void spMeshRenderFeatureExtractor::Extract(spSceneContext* pSceneContext, const spRenderContext* pRenderContext, const spRenderView* pRenderView)
  {
    spVisibilityGroup visibilityGroup(pSceneContext);

    spExtractMeshRenderObjectMessage message;
    message.m_pRenderContext = pRenderContext;
    message.m_pRenderView = pRenderView;
    message.m_Objects = EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spRenderObjectCollection, &visibilityGroup);

    // TODO: Handle caching if game object is static or render object caching behavior is set to always
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshRenderFeatureExtractor);
