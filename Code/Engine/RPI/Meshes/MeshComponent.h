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

#pragma once

#include <RPI/RPIDLL.h>

#include <RAI/Resources/MeshResource.h>

#include <RPI/Features/RenderComponent.h>
#include <RPI/Meshes/MeshRenderFeature.h>

namespace RPI
{
  class SP_RPI_DLL spMeshComponentManager : public ezComponentManager<class spMeshComponent, ezBlockStorageType::FreeList>
  {
  public:
    using SUPER = ezComponentManager<class spMeshComponent, ezBlockStorageType::FreeList>;

    explicit spMeshComponentManager(ezWorld* pWorld);
    ~spMeshComponentManager() override;

  protected:
    void Initialize() override;
    void Deinitialize() override;

  private:
    ezUniquePtr<spMeshRenderFeature> m_pRenderFeature{nullptr};
  };

  class SP_RPI_DLL spMeshComponent : public spRenderComponent
  {
    EZ_DECLARE_COMPONENT_TYPE(spMeshComponent, spRenderComponent, spMeshComponentManager);

    // ezComponent

  public:
    void SerializeComponent(ezWorldWriter& inout_stream) const override;
    void DeserializeComponent(ezWorldReader& in_stream) override;

    // spRenderComponent

  public:
    ezResult GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible) const override;

    // spMeshComponent

  public:
    spMeshComponent();
    virtual ~spMeshComponent() = default;

#pragma region Methods

    void SetMesh(const RAI::spMeshResourceHandle& hMeshResource);
    EZ_ALWAYS_INLINE const RAI::spMeshResourceHandle& GetMesh() const { return m_hMeshResource; }

#pragma region Messages

    void OnMsgExtract(spExtractMeshRenderObjectMessage& ref_msg) const;

#pragma endregion

#pragma region Properties

    void SetMeshFile(const char* szMeshFile);
    [[nodiscard]] const char* GetMeshFile() const;

    void SetSortingOrder(float fSortingOrder);
    [[nodiscard]] float GetSortingOrder() const;

#pragma endregion

  private:
    RAI::spMeshResourceHandle m_hMeshResource;
    float m_fSortingOrder{0.0f};
  };
} // namespace RPI
