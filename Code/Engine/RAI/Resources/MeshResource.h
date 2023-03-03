#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Core.h>
#include <RAI/Mesh.h>

#include <Core/ResourceManager/Resource.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/Math/BoundingBoxSphere.h>

namespace RAI
{
  typedef ezTypedResourceHandle<class spMeshResource> spMeshResourceHandle;

  class SP_RAI_DLL spMeshResourceDescriptor
  {
    friend class spMeshResource;

  public:
    spMeshResourceDescriptor();

    void Clear();

    EZ_NODISCARD const spMesh& GetLOD(ezUInt32 uiLodIndex = 0) const;

    EZ_NODISCARD spMesh& WriteLOD(ezUInt32 uiLodIndex = 0);

    void ClearLOD(ezUInt32 uiLodIndex = 0);

    void SetLOD(ezUInt32 uiLodIndex, const spMesh& mesh);

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt8 GetNumLODs() const { return m_uiNumLOD; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFile);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFile);

  private:
    ezUInt8 m_uiNumLOD{0};

    ezStaticArray<spMesh, SP_RAI_MAX_LOD_COUNT> m_LODs;

    ezBoundingBoxSphere m_Bounds;
  };

  class SP_RAI_DLL spMeshResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spMeshResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spMeshResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spMeshResource, spMeshResourceDescriptor);

  public:
    spMeshResource();

    EZ_NODISCARD EZ_ALWAYS_INLINE const spMeshResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spMeshResourceDescriptor m_Descriptor;
    ezBoundingBoxSphere m_Bounds;
  };
} // namespace RAI
