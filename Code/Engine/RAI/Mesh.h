#pragma once

#include <RAI/RAIDLL.h>

/// \brief A mesh asset. Stores all needed data to render a mesh.
class SP_RAI_DLL spMesh
{
  friend class spMeshResource;
  friend class spMeshResourceDescriptor;

public:
  /// \brief A single vertex in the mesh asset.
  struct Vertex
  {
    EZ_DECLARE_POD_TYPE();

    /// \brief The vertex position.
    ezVec3 m_vPosition;

    /// \brief The vertex normal.
    ezVec3 m_vNormal;

    /// \brief The vertex tangent.
    ezVec4 m_vTangent;

    /// \brief The vertex bitangent.
    ezVec4 m_vBiTangent;

    /// \brief The vertex texture coordinates at channel 1.
    ezVec2 m_vTexCoord0;

    /// \brief The vertex texture coordinates at channel 2.
    ezVec2 m_vTexCoord1;

    /// \brief The vertex color at channel 1.
    ezColor m_Color0;

    /// \brief The vertex color at channel 2.
    ezColor m_Color1;
  };

  /// \brief The mesh data. Stores all the vertices and indices.
  struct Data
  {
    /// \brief The mesh vertices.
    ezDynamicArray<Vertex> m_Vertices;

    /// \brief The mesh indices.
    ezDynamicArray<ezUInt16> m_Indices;
  };

  /// \brief An entry (sub-mesh) in the mesh.
  struct Entry
  {
    /// \brief The sub-mesh name.
    ezString m_sName;

    /// \brief The offset from the beginning of the indices
    /// buffer telling where this sub-mesh data live in the mesh \a Data.
    ezUInt32 m_uiBaseIndex{0};

    /// \brief The number of indices to read starting at the offset when drawing the sub-mesh.
    ezUInt32 m_uiIndicesCount{0};

    /// \brief The offset from the beginning of the vertices
    /// buffer telling where this sub-mesh data live in the mesh \a Data.
    ezUInt32 m_uiBaseVertex{0};

    /// \brief The number of vertices to read starting at the offset when drawing the sub-mesh.
    ezUInt32 m_uiVerticesCount{0};
  };

  /// \brief Stores transformation data for mesh nodes.
  struct Transform
  {
    EZ_DECLARE_POD_TYPE();

    /// \brief The node position.
    ezVec3 m_vPosition;

    /// \brief The node scale.
    ezVec3 m_vScale;

    /// \brief The node rotation.
    ezVec3 m_vRotation;
  };

  /// \brief A mesh node.
  ///
  /// This is to maintain the same hierarchy as in the DCC tool. A node
  /// may not have mesh data, therefore the entries array will be empty.
  struct Node
  {
    /// \brief The node name.
    ezString m_sName;

    /// \brief The list of meshes in this node.
    ezDynamicArray<Entry> m_Entries;

    /// \brief The node children.
    ezDynamicArray<Node> m_Children;

    /// \brief The node transformation.
    Transform m_Transform;

    /// \brief The name of the material applied to this node.
    ezString m_sMaterial;
  };

  /// \brief Default constructor.
  spMesh() = default;

  /// \brief Creates a new mesh asset from the given \a meshData and \a rootNode.
  /// \param meshData The mesh data.
  /// \param rootNode The root node.
  spMesh(Data meshData, Node rootNode)
    : m_Data(std::move(meshData))
    , m_Root(std::move(rootNode))
  {
  }

  /// \brief Gets the data of this mesh asset.
  EZ_NODISCARD EZ_ALWAYS_INLINE const Data& GetData() const { return m_Data; }

  /// \brief Sets the data of this mesh asset.
  /// \param [in] data The mesh data.
  EZ_ALWAYS_INLINE void SetData(const Data& data) { m_Data = data; }

  /// \brief Gets the root node of this mesh asset.
  EZ_NODISCARD EZ_ALWAYS_INLINE const Node& GetRootNode() const { return m_Root; }

  /// \brief Sets the root node of this mesh asset.
  /// \param [in] rootNode The root node.
  EZ_ALWAYS_INLINE void SetRootNode(const Node& rootNode) { m_Root = rootNode; }

  /// \brief Clear the mesh data.
  void Clear();

private:
  Data m_Data;
  Node m_Root;
};

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spMesh::Vertex& vertex)
{
  inout_stream << vertex.m_vPosition;
  inout_stream << vertex.m_vNormal;
  inout_stream << vertex.m_vTangent;
  inout_stream << vertex.m_vBiTangent;
  inout_stream << vertex.m_vTexCoord0;
  inout_stream << vertex.m_vTexCoord1;
  inout_stream << vertex.m_Color0;
  inout_stream << vertex.m_Color1;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, spMesh::Vertex& ref_vertex)
{
  inout_stream >> ref_vertex.m_vPosition;
  inout_stream >> ref_vertex.m_vNormal;
  inout_stream >> ref_vertex.m_vTangent;
  inout_stream >> ref_vertex.m_vBiTangent;
  inout_stream >> ref_vertex.m_vTexCoord0;
  inout_stream >> ref_vertex.m_vTexCoord1;
  inout_stream >> ref_vertex.m_Color0;
  inout_stream >> ref_vertex.m_Color1;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spMesh::Transform& transform)
{
  inout_stream << transform.m_vPosition;
  inout_stream << transform.m_vScale;
  inout_stream << transform.m_vRotation;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, spMesh::Transform& ref_transform)
{
  inout_stream >> ref_transform.m_vPosition;
  inout_stream >> ref_transform.m_vScale;
  inout_stream >> ref_transform.m_vRotation;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spMesh::Entry& meshEntry)
{
  inout_stream << meshEntry.m_sName;
  inout_stream << meshEntry.m_uiBaseIndex;
  inout_stream << meshEntry.m_uiIndicesCount;
  inout_stream << meshEntry.m_uiBaseVertex;
  inout_stream << meshEntry.m_uiVerticesCount;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, spMesh::Entry& ref_meshEntry)
{
  inout_stream >> ref_meshEntry.m_sName;
  inout_stream >> ref_meshEntry.m_uiBaseIndex;
  inout_stream >> ref_meshEntry.m_uiIndicesCount;
  inout_stream >> ref_meshEntry.m_uiBaseVertex;
  inout_stream >> ref_meshEntry.m_uiVerticesCount;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spMesh::Node& meshNode)
{
  inout_stream << meshNode.m_sName;
  inout_stream << meshNode.m_Transform;
  inout_stream << meshNode.m_sMaterial;
  inout_stream.WriteArray(meshNode.m_Entries).AssertSuccess();
  inout_stream.WriteArray(meshNode.m_Children).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, spMesh::Node& ref_meshNode)
{
  inout_stream >> ref_meshNode.m_sName;
  inout_stream >> ref_meshNode.m_Transform;
  inout_stream >> ref_meshNode.m_sMaterial;
  inout_stream.ReadArray(ref_meshNode.m_Entries).AssertSuccess();
  inout_stream.ReadArray(ref_meshNode.m_Children).AssertSuccess();

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spMesh::Data& meshData)
{
  inout_stream.WriteArray(meshData.m_Vertices).AssertSuccess();
  inout_stream.WriteArray(meshData.m_Indices).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, spMesh::Data& ref_meshData)
{
  inout_stream.ReadArray(ref_meshData.m_Vertices).AssertSuccess();
  inout_stream.ReadArray(ref_meshData.m_Indices).AssertSuccess();

  return inout_stream;
}
