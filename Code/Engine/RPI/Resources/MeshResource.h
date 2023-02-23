#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Assets/Mesh.h>
#include <RPI/Core.h>

#include <Core/ResourceManager/Resource.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/Math/BoundingBoxSphere.h>

typedef ezTypedResourceHandle<class spMeshResource> spMeshResourceHandle;

class SP_RPI_DLL spMeshResourceDescriptor
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
  static void WriteMeshData(ezChunkStreamWriter& chunk, const spMesh& mesh);
  static void ReadMeshData(ezChunkStreamReader& chunk, spMesh& mesh);
  static void WriteMeshNode(ezChunkStreamWriter& chunk, const spMesh::Node& node);
  static void ReadMeshNode(ezChunkStreamReader& chunk, spMesh::Node& node);

  ezUInt32 m_uiNumLOD{0};

  spMesh m_LOD0;

  ezBoundingBoxSphere m_Bounds;
};

class SP_RPI_DLL spMeshResource final : public ezResource
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
