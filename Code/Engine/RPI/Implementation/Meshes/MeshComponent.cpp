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
      EZ_ACCESSOR_PROPERTY("MaxDistance", GetLODMaxDistance, SetLODMaxDistance)->AddAttributes(new ezGroupAttribute("Level Of Detail"), new ezClampValueAttribute(0.0f, {})),
      EZ_ENUM_ACCESSOR_PROPERTY("FetchFunction", spMeshLevelOfDetailFetchFunction, GetLODFetchFunction, SetLODFetchFunction)->AddAttributes(new ezGroupAttribute("Level Of Detail")),
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
    m_pRenderFeature.Clear();
  }

  void spMeshComponentManager::Initialize()
  {
    SUPER::Initialize();

    const spSceneContext* context = spRenderSystem::GetSingleton()->GetSceneContextFromWorld(GetWorld());
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
    if (m_RenderObject.m_hMeshResource == hMeshResource)
      return;

    m_RenderObject.m_hMeshResource = hMeshResource;

    TriggerLocalBoundsUpdate();
  }

  void spMeshComponent::SetMaterial(const spMaterialResourceHandle& hMaterialResource)
  {
    if (m_RenderObject.m_hMaterialResource == hMaterialResource)
      return;

    ezResourceLock pMaterialResource(hMaterialResource, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (pMaterialResource.GetAcquireResult() == ezResourceAcquireResult::Final)
    {
      m_RenderObject.m_hMaterialResource = hMaterialResource;
      m_RenderObject.m_hRootMaterialResource = pMaterialResource->GetDescriptor().GetRootMaterialResource();
    }
  }

  void spMeshComponent::SerializeComponent(ezWorldWriter& inout_stream) const
  {
    spRenderComponent::SerializeComponent(inout_stream);
    ezStreamWriter& stream = inout_stream.GetStream();

    stream.WriteVersion(kMeshComponentVersion);

    stream << m_RenderObject.m_hMeshResource;
  }

  void spMeshComponent::DeserializeComponent(ezWorldReader& in_stream)
  {
    ezComponent::DeserializeComponent(in_stream);
    ezStreamReader& stream = in_stream.GetStream();

    stream.ReadVersion(kMeshComponentVersion);

    stream >> m_RenderObject.m_hMeshResource;
  }

  ezResult spMeshComponent::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible)
  {
    if (m_RenderObject.m_hMeshResource.IsValid())
    {
      const ezMat4 mTransform = GetOwner()->GetGlobalTransform().GetAsMat4();

      ezResourceLock pMesh(m_RenderObject.m_hMeshResource, ezResourceAcquireMode::AllowLoadingFallback);

      const auto bounds = pMesh->GetLOD(0).GetBounds().GetBox();

      ref_bounds = ezBoundingBox::MakeFromMinMax(
        mTransform.TransformPosition(bounds.m_vMin),
        mTransform.TransformPosition(bounds.m_vMax));

      return EZ_SUCCESS;
    }

    return EZ_FAILURE;
  }

  void spMeshComponent::OnMsgExtract(spExtractMeshRenderObjectMessage& ref_msg)
  {
    if (!m_RenderObject.m_hMeshResource.IsValid())
      return;

    m_RenderObject.m_BoundingBox = GetOwner()->GetLocalBounds();
    m_RenderObject.m_Transform = GetOwner()->GetGlobalTransform();
    m_RenderObject.m_PreviousTransform = GetOwner()->GetLastGlobalTransform();
    m_RenderObject.m_eRenderGroup = spRenderGroup::All;
    m_RenderObject.m_uiUniqueID = GetHandle().GetInternalID().m_Data;

    ref_msg.m_Objects->Add(&m_RenderObject);
  }

  void spMeshComponent::SetMeshFile(const char* szMeshFile)
  {
    RAI::spMeshResourceHandle hMeshResource;

    if (!ezStringUtils::IsNullOrEmpty(szMeshFile))
      hMeshResource = ezResourceManager::LoadResource<RAI::spMeshResource>(szMeshFile);

    SetMesh(hMeshResource);
  }

  const char* spMeshComponent::GetMeshFile() const
  {
    ezStringBuilder sTemp;
    return m_RenderObject.m_hMeshResource.IsValid()
             ? m_RenderObject.m_hMeshResource.GetResourceID().GetData(sTemp)
             : "";
  }

  void spMeshComponent::SetMaterialFile(const char* szMaterialFile)
  {
    spMaterialResourceHandle hMaterialResource;

    if (!ezStringUtils::IsNullOrEmpty(szMaterialFile))
      hMaterialResource = ezResourceManager::LoadResource<spMaterialResource>(szMaterialFile);

    SetMaterial(hMaterialResource);
  }

  const char* spMeshComponent::GetMaterialFile() const
  {
    ezStringBuilder sTemp;
    return m_RenderObject.m_hMaterialResource.IsValid()
             ? m_RenderObject.m_hMaterialResource.GetResourceID().GetData(sTemp)
             : "";
  }

  void spMeshComponent::SetLODMaxDistance(float fMaxDistance)
  {
    m_RenderObject.m_fLODMaxDistance = fMaxDistance;
  }

  float spMeshComponent::GetLODMaxDistance() const
  {
    return m_RenderObject.m_fLODMaxDistance;
  }

  void spMeshComponent::SetLODFetchFunction(ezEnum<spMeshLevelOfDetailFetchFunction> eFetchFunction)
  {
    m_RenderObject.m_eLODFetchFunction = eFetchFunction;
  }

  ezEnum<spMeshLevelOfDetailFetchFunction> spMeshComponent::GetLODFetchFunction() const
  {
    return m_RenderObject.m_eLODFetchFunction;
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshComponent);
