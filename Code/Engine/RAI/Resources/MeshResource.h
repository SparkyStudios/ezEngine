#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Core.h>
#include <RAI/Mesh.h>

#include <Core/ResourceManager/Resource.h>

#include <Foundation/Math/BoundingBoxSphere.h>

namespace RAI
{
  /// \brief Handle for a mesh resource.
  typedef ezTypedResourceHandle<class spMeshResource> spMeshResourceHandle;

  /// \brief A mesh resource descriptor.
  ///
  /// This descriptor contains all the information needed to create a mesh resource. It supports
  /// a basic LOD system where each LOD is a separate \a spMesh instance. The LOD system
  /// can support a maximum of \a SP_RAI_MAX_LOD_COUNT LODs with a default value of 5.
  class SP_RAI_DLL spMeshResourceDescriptor
  {
    friend class spMeshResource;

  public:
    spMeshResourceDescriptor();

    /// \brief Clears the descriptor.
    void Clear();

    /// \brief Gets the LOD at the given index.
    /// \note An assertion will be thrown if the index is out of bounds during development.
    /// Therefore, this method will crash on production if called with a wrong index.
    /// \param uiLodIndex The index of the LOD to get.
    EZ_NODISCARD const spMesh& GetLOD(ezUInt32 uiLodIndex = 0) const;

    /// \brief Gets the LOD at the given index.
    /// \note An assertion will be thrown if the index is out of bounds during development.
    /// Therefore, this method will crash on production if called with a wrong index.
    /// \param uiLodIndex The index of the LOD to get.
    spMesh& GetLOD(ezUInt32 uiLodIndex = 0);

    /// \brief Gets the address of the LOD at the given index.
    /// \note An assertion will be thrown if the index is out of bounds during development.
    /// Therefore, this method will crash on production if called with a wrong index.
    /// \param uiLodIndex The index of the LOD to write.
    EZ_NODISCARD spMesh& WriteLOD(ezUInt32 uiLodIndex = 0);

    /// \brief Clears the LOD data at the given index, without removing the LOD itself.
    /// \param uiLodIndex The index of the LOD to clear.
    void ClearLOD(ezUInt32 uiLodIndex = 0);

    /// \brief Sets the LOD of the given index.
    /// \param  uiLodIndex The index where to set the given LOD mesh.
    /// \param mesh The LOD mesh.
    void SetLOD(ezUInt32 uiLodIndex, const spMesh& mesh);

    /// \brief Gets the number of LODs currently set in the mesh.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt8 GetNumLODs() const { return m_uiNumLOD; }

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
    ezUInt8 m_uiNumLOD{0};

    ezStaticArray<spMesh, SP_RAI_MAX_LOD_COUNT> m_LODs;

    ezBoundingBoxSphere m_Bounds;
  };

  /// \brief A resource encapsulating a mesh asset.
  class SP_RAI_DLL spMeshResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spMeshResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spMeshResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spMeshResource, spMeshResourceDescriptor);

  public:
    spMeshResource();

    EZ_NODISCARD EZ_ALWAYS_INLINE const spMeshResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

    /// \brief Gets the LOD at the given index.
    /// \note An assertion will be thrown if the index is out of bounds during development.
    /// Therefore, this method will crash on production if called with a wrong index.
    /// \param uiLodIndex The index of the LOD to get.
    EZ_NODISCARD EZ_ALWAYS_INLINE const spMesh& GetLOD(ezUInt32 uiLodIndex = 0) const { return m_Descriptor.GetLOD(uiLodIndex); }

    /// \brief Gets the LOD at the given index.
    /// \note An assertion will be thrown if the index is out of bounds during development.
    /// Therefore, this method will crash on production if called with a wrong index.
    /// \param uiLodIndex The index of the LOD to get.
    EZ_ALWAYS_INLINE spMesh& GetLOD(ezUInt32 uiLodIndex = 0) { return m_Descriptor.GetLOD(uiLodIndex); }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spMeshResourceDescriptor m_Descriptor;
    ezBoundingBoxSphere m_Bounds;
  };
} // namespace RAI
