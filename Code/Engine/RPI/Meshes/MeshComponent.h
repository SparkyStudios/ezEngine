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

#include <RPI/Resources/MeshResource.h>

#include <RPI/Features/RenderComponent.h>
#include <RPI/Meshes/MeshRenderFeature.h>
#include <RPI/Resources/MaterialResource.h>

namespace RPI
{
  class SP_RPI_DLL spMeshComponentManager : public ezComponentManager<class spMeshComponent, ezBlockStorageType::FreeList>
  {
  public:
    using SUPER = ezComponentManager<class spMeshComponent, ezBlockStorageType::FreeList>;

    explicit spMeshComponentManager(ezWorld* pWorld);
    ~spMeshComponentManager() override;

    [[nodiscard]] EZ_ALWAYS_INLINE spMeshRenderFeature* GetMeshRenderFeature() const { return m_pRenderFeature.Borrow(); }

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
    void SetMesh(const spMeshResourceHandle& hMeshResource);

    /// \brief Returns the handle to the mesh resource to be rendered.
    EZ_ALWAYS_INLINE const spMeshResourceHandle& GetMesh() const { return m_RenderObject.m_hMeshResource; }

    /// \brief Sets the handle to the material resource to be used for rendering.
    /// \param hMaterialResource The handle to the material resource to be used for rendering.
    void SetMaterial(const spMaterialResourceHandle& hMaterialResource);

    /// \brief Returns the handle to the material resource to be used for rendering.
    EZ_ALWAYS_INLINE const spMaterialResourceHandle& GetMaterial() const { return m_RenderObject.m_hMaterialResource; }

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
    [[nodiscard]] const char* GetMeshFile() const;

    /// \brief Sets the material file for the mesh.
    /// \param szMaterialFile The path to the material file.
    void SetMaterialFile(const char* szMaterialFile);

    /// \brief Returns the path to the material file of the mesh.
    [[nodiscard]] const char* GetMaterialFile() const;

    /// \brief Sets the max distance at which the mesh will be rendered with
    /// the lowest level of detail.
    /// \param fMaxDistance The max distance.
    void SetLODMaxDistance(float fMaxDistance);

    /// \brief Returns the max distance at which the mesh will be rendered with
    /// the lowest level of detail.
    [[nodiscard]] float GetLODMaxDistance() const;

    /// \brief Sets the LOD fetch function.
    /// \param eFetchFunction The LOD fetch function.
    void SetLODFetchFunction(ezEnum<spMeshLevelOfDetailFetchFunction> eFetchFunction);

    /// \brief Returns the LOD fetch function.
    [[nodiscard]] ezEnum<spMeshLevelOfDetailFetchFunction> GetLODFetchFunction() const;

#pragma endregion

  private:
    spMeshRenderObject m_RenderObject;
  };
} // namespace RPI
