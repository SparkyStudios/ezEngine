#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Mesh.h>
#include <RAI/Core.h>

#include <Core/ResourceManager/Resource.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/Math/BoundingBoxSphere.h>

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

  ezResult Save(ezStreamWriter& stream);
  ezResult Save(ezStringView sFile);

  ezResult Load(ezStreamReader& inout_stream);
  ezResult Load(ezStringView sFile);

private:
  ezUInt32 m_uiNumLOD{0};

  spMesh m_LOD0;

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

  EZ_NODISCARD EZ_ALWAYS_INLINE ezArrayPtr<const spMesh::Vertex> GetVertices() const { return m_Descriptor.GetLOD(0).GetData().m_Vertices; }

  EZ_NODISCARD EZ_ALWAYS_INLINE ezArrayPtr<const ezUInt16> GetIndices() const { return m_Descriptor.GetLOD(0).GetData().m_Indices; }

private:
  ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
  void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  spMeshResourceDescriptor m_Descriptor;
  ezBoundingBoxSphere m_Bounds;
};
