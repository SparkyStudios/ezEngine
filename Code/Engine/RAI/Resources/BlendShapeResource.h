#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/BlendShape.h>

#include <Core/ResourceManager/Resource.h>

namespace RAI
{
  /// \brief Handle for
  typedef ezTypedResourceHandle<class spBlendShapeResource> spBlendShapeResourceHandle;

  /// \brief A blend shape resource descriptor.
  ///
  /// This descriptor contains all the blend shapes associated to a mesh resource. According
  /// to the way it has been designed in the DCC tool, it is possible to use the same blend
  /// shape resource for multiple meshes resources.
  class SP_RAI_DLL spBlendShapeResourceDescriptor
  {
    friend class spBlendShapeResource;

  public:
    spBlendShapeResourceDescriptor();

    /// \brief Clears the descriptor.
    void Clear();

    EZ_NODISCARD ezArrayPtr<const spBlendShape> GetBlendShapes(ezStringView sMeshName) const;

    EZ_NODISCARD bool GetBlendShape(ezStringView sMeshName, ezStringView sName, const spBlendShape*& out_blendShape) const;

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
  class SP_RAI_DLL spBlendShapeResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spBlendShapeResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spBlendShapeResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spBlendShapeResource, spBlendShapeResourceDescriptor);

  public:
    spBlendShapeResource();

    EZ_NODISCARD EZ_ALWAYS_INLINE const spBlendShapeResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spBlendShapeResourceDescriptor m_Descriptor;
  };
} // namespace RAI
