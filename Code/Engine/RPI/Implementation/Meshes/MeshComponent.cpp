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

#include <RPI/Meshes/MeshComponent.h>
#include <RPI/Scene/SceneContext.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

namespace RPI
{
  constexpr ezTypeVersion kMeshComponentVersion = 1;

  // clang-format off
  EZ_BEGIN_COMPONENT_TYPE(spMeshComponent, kMeshComponentVersion, ezComponentMode::Static)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_ACCESSOR_PROPERTY("Mesh", GetMeshFile, SetMeshFile)->AddAttributes(new ezAssetBrowserAttribute("*.spMesh")),
      EZ_ACCESSOR_PROPERTY("SortingOrder", GetSortingOrder, SetSortingOrder)
    }
    EZ_END_PROPERTIES;

    EZ_BEGIN_MESSAGEHANDLERS
    {
      EZ_MESSAGE_HANDLER(spExtractMeshRenderObjectMessage, OnMsgExtract),
    }
    EZ_END_MESSAGEHANDLERS;
  }
  EZ_END_COMPONENT_TYPE;
  // clang-format on

  spMeshComponentManager::spMeshComponentManager(ezWorld* pWorld)
    : ezComponentManager(pWorld)
  {
    m_pRenderFeature = EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spMeshRenderFeature);
  }

  spMeshComponentManager::~spMeshComponentManager()
  {
  }

  void spMeshComponentManager::Initialize()
  {
    SUPER::Initialize();

    spSceneContext* context = spRenderSystem::GetSingleton()->GetSceneContextFromWorld(GetWorld());
    if (context == nullptr)
    {
      ezLog::Warning("Cannot initialize mesh render feature, no scene context available for the current world.");
      return;
    }

    m_pRenderFeature->Initialize(context->GetRenderContext());
  }

  void spMeshComponentManager::Deinitialize()
  {
    SUPER::Deinitialize();

    m_pRenderFeature->Deinitialize();
  }

  spMeshComponent::spMeshComponent()
    : spRenderComponent(ezGetStaticRTTI<spMeshRenderFeature>())
  {
  }

  void spMeshComponent::SetMesh(const RAI::spMeshResourceHandle& hMeshResource)
  {
    if (m_hMeshResource == hMeshResource)
      return;

    m_hMeshResource = hMeshResource;

    TriggerLocalBoundsUpdate();
  }

  void spMeshComponent::SerializeComponent(ezWorldWriter& inout_stream) const
  {
    spRenderComponent::SerializeComponent(inout_stream);
    ezStreamWriter& stream = inout_stream.GetStream();

    stream.WriteVersion(kMeshComponentVersion);

    stream << m_hMeshResource;
    stream << m_fSortingOrder;
  }

  void spMeshComponent::DeserializeComponent(ezWorldReader& in_stream)
  {
    ezComponent::DeserializeComponent(in_stream);
    ezStreamReader& stream = in_stream.GetStream();

    stream.ReadVersion(kMeshComponentVersion);

    stream >> m_hMeshResource;
    stream >> m_fSortingOrder;
  }

  ezResult spMeshComponent::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible) const
  {
    if (m_hMeshResource.IsValid())
    {
      ezResourceLock<RAI::spMeshResource> pMesh(m_hMeshResource, ezResourceAcquireMode::AllowLoadingFallback);
      ref_bounds = pMesh->GetLOD(0).GetBounds();
      return EZ_SUCCESS;
    }

    return EZ_FAILURE;
  }

  void spMeshComponent::OnMsgExtract(spExtractMeshRenderObjectMessage& ref_msg) const
  {
    // TODO
  }

  void spMeshComponent::SetMeshFile(const char* szMeshFile)
  {
    RAI::spMeshResourceHandle hMeshResource;

    if (!ezStringUtils::IsNullOrEmpty(szMeshFile))
    {
      hMeshResource = ezResourceManager::LoadResource<RAI::spMeshResource>(szMeshFile);
    }

    SetMesh(hMeshResource);
  }

  const char* spMeshComponent::GetMeshFile() const
  {
    return m_hMeshResource.IsValid() ? m_hMeshResource.GetResourceID() : "";
  }

  void spMeshComponent::SetSortingOrder(float fSortingOrder)
  {
    m_fSortingOrder = fSortingOrder;
  }

  float spMeshComponent::GetSortingOrder() const
  {
    return m_fSortingOrder;
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshComponent);
