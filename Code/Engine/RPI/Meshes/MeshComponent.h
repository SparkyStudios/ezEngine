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

    EZ_NODISCARD EZ_ALWAYS_INLINE spMeshRenderFeature* GetMeshRenderFeature() const { return m_pRenderFeature.Borrow(); }

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
    ezResult GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible) override;

    // spMeshComponent

  public:
    spMeshComponent();
    ~spMeshComponent() override = default;

#pragma region Methods

  public:
    /// \brief Sets the handle to the mesh resource to be rendered.
    /// \param hMeshResource The handle to the mesh resource to be rendered.
    void SetMesh(const RAI::spMeshResourceHandle& hMeshResource);

    /// \brief Returns the handle to the mesh resource to be rendered.
    EZ_ALWAYS_INLINE const RAI::spMeshResourceHandle& GetMesh() const { return m_RenderObject.m_hMeshResource; }

#pragma endregion

#pragma region Messages

  private:
    /// \brief Called when the \a spMeshRenderFeature want to extract render data from this mesh.
    /// \param ref_msg The extraction message.
    void OnMsgExtract(spExtractMeshRenderObjectMessage& ref_msg);

#pragma endregion

#pragma region Properties

  public:
    /// \brief Sets the mesh file to be rendered.
    /// \param szMeshFile The path to the mesh file to be rendered.
    void SetMeshFile(const char* szMeshFile);

    /// \brief Returns the path to the mesh file to be rendered.
    EZ_NODISCARD const char* GetMeshFile() const;

#pragma endregion

  private:
    spMeshRenderObject m_RenderObject;
  };
} // namespace RPI
