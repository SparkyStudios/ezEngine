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

#include <RPI/Core/RenderObject.h>
#include <RPI/Resources/MaterialResource.h>
#include <RPI/Shaders/ShaderTypes.h>

#include <RPI/Resources/MeshResource.h>

namespace RPI
{
  /// \brief Specifies the function used to fetch the level of detail of a mesh.
  struct SP_RPI_DLL spMeshLevelOfDetailFetchFunction
  {
    typedef ezUInt8 StorageType;

    enum Enum : StorageType
    {
      /// \brief The distance between each LOD is constant.
      Constant,

      /// \brief The distance between each LOD is logarithmic.
      Logarithmic,

      /// \brief The distance between each LOD is exponential.
      Exponential,

      Default = Constant,
    };
  };

  // TODO: Find a better place for this.
  struct alignas(16) spInstanceData
  {
    EZ_DECLARE_POD_TYPE();

    spShaderTransform m_Transform{ezTransform::MakeIdentity()};
    spShaderTransform m_PreviousTransform{ezTransform::MakeIdentity()};

    spShaderTransform m_NormalTransform{ezTransform::MakeIdentity()};

    spShaderVec3 _padding0;
    ezUInt32 m_MaterialIndex{0};
  };

  class SP_RPI_DLL spMeshRenderObject final : public spRenderObject
  {
    friend class spMeshComponent;
    friend class spMeshRenderFeature;
    friend class spMeshRenderFeatureExtractor;

    EZ_ADD_DYNAMIC_REFLECTION(spMeshRenderObject, spRenderObject);

    // spRenderObject

  public:
    bool CanBeInstanceOf(spRenderObject* pRenderObject) const override;
    ezResult Instantiate(spRenderObject* pRenderObject) override;
    void MakeRootInstance() override;
    bool HasInstances() override;

  private:
    void FillInstanceData(spInstanceData& instance) const;
    ezUInt32 FillMaterialData(spMaterialResourceHandle hMaterialResource);

    [[nodiscard]] ezUInt32 GetInstanceBufferSize() const;
    [[nodiscard]] ezUInt32 GetMaterialBufferSize() const;

    void CreateInstanceBuffer();
    void CreateMaterialBuffer();
    void UpdateBuffer();

    spMeshResourceHandle m_hMeshResource;
    spMaterialResourceHandle m_hMaterialResource;
    spRootMaterialResourceHandle m_hRootMaterialResource;

    ezTransform m_Transform{ezTransform::MakeIdentity()};
    ezTransform m_PreviousTransform{ezTransform::MakeIdentity()};
    float m_fLODMaxDistance{1000.0f};
    ezEnum<spMeshLevelOfDetailFetchFunction> m_eLODFetchFunction{spMeshLevelOfDetailFetchFunction::Default};

    ezArrayMap<ezUInt64, ezUInt32> m_Instances;

    ezDynamicArray<spInstanceData, ezAlignedAllocatorWrapper> m_PerInstanceData;
    ezDynamicArray<spMaterialData, ezAlignedAllocatorWrapper> m_PerMaterialData;

    ezSharedPtr<RHI::spBuffer> m_pPerInstanceDataBuffer;
    ezSharedPtr<RHI::spBuffer> m_pPerMaterialDataBuffer;

    ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper> m_DrawCommands;
    ezSharedPtr<RHI::spBuffer> m_pIndirectBuffer;
    bool m_bIndirectBufferDirty{true};
  };
} // namespace RPI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spMeshLevelOfDetailFetchFunction);
