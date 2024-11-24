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

#include <RPI/Assets/BlendShape.h>

#include <Core/ResourceManager/Resource.h>

namespace RPI
{
  /// \brief Handle for a \a spBlendShapeResource
  typedef ezTypedResourceHandle<class spBlendShapeResource> spBlendShapeResourceHandle;

  /// \brief A blend shape resource descriptor.
  ///
  /// This descriptor contains all the blend shapes associated to a mesh resource. According
  /// to the way it has been designed in the DCC tool, it is possible to use the same blend
  /// shape resource for multiple meshes resources.
  class SP_RPI_DLL spBlendShapeResourceDescriptor
  {
    friend class spBlendShapeResource;

  public:
    spBlendShapeResourceDescriptor();

    /// \brief Clears the descriptor.
    void Clear();

    [[nodiscard]] ezArrayPtr<const spBlendShape> GetBlendShapes(ezStringView sMeshName) const;

    [[nodiscard]] bool GetBlendShape(ezStringView sMeshName, ezStringView sName, const spBlendShape*& out_blendShape) const;

    void AddBlendShape(ezStringView sMeshName, const spBlendShape& blendShape);

    /// \brief Writes the mesh asset in the given stream.
    /// \param inout_stream The stream in which the mesh asset will be written.
    ezResult Save(ezStreamWriter& inout_stream);

    /// \brief Loads a mesh asset from the given stream.
    /// \param inout_stream The stream from which the mesh asset will be loaded.
    ezResult Load(ezStreamReader& inout_stream);

    /// \brief Loafs a mesh asset from the given file.
    /// \param sFileName The path to the serialized mesh asset file.
    ezResult Load(ezStringView sFileName);

  private:
    ezArrayMap<ezHashedString, spBlendShape> m_BlendShapes;
  };

  /// \brief A resource encapsulating a blend shape asset.
  class SP_RPI_DLL spBlendShapeResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spBlendShapeResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spBlendShapeResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spBlendShapeResource, spBlendShapeResourceDescriptor);

  public:
    spBlendShapeResource();

    [[nodiscard]] EZ_ALWAYS_INLINE const spBlendShapeResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spBlendShapeResourceDescriptor m_Descriptor;
  };
} // namespace RPI
