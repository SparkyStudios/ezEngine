#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Core.h>

#include <meshoptimizer.h>

namespace RAI
{
  /// \brief A mesh asset. Stores all needed data to render a mesh.
  class SP_RAI_DLL spMesh
  {
    friend class spMeshResource;
    friend class spMeshResourceDescriptor;

  public:
    /// \brief The mesh data. Stores all the vertices and indices.
    struct Data
    {
      /// \brief The mesh vertices.
      ezDynamicArray<spVertex> m_Vertices;

      /// \brief The mesh indices.
      ezDynamicArray<ezUInt16> m_Indices;
    };

    /// \brief An entry (sub-mesh) in the mesh.
    struct Entry
    {
      /// \brief The sub-mesh name.
      ezHashedString m_sName;

      /// \brief The offset from the beginning of the indices
      /// buffer telling where this sub-mesh data live in the mesh \a Data.
      ezUInt32 m_uiBaseIndex{0};

      /// \brief The number of indices to read starting at the offset when drawing the sub-mesh.
      ezUInt32 m_uiIndexCount{0};

      /// \brief The offset from the beginning of the vertices
      /// buffer telling where this sub-mesh data live in the mesh \a Data.
      ezUInt32 m_uiBaseVertex{0};

      /// \brief The number of vertices to read starting at the offset when drawing the sub-mesh.
      ezUInt32 m_uiVertexCount{0};
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
      ezHashedString m_sName;

      /// \brief The list of meshes in this node.
      ezDynamicArray<Entry> m_Entries;

      /// \brief The node children.
      ezDynamicArray<Node> m_Children;

      /// \brief The node transformation.
      Transform m_Transform;

      /// \brief The name of the material applied to this node.
      ezHashedString m_sMaterial;
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

    /// \brief Gets the data of this mesh asset.
    EZ_ALWAYS_INLINE Data& GetData() { return m_Data; }

    /// \brief Sets the data of this mesh asset.
    /// \param [in] data The mesh data.
    EZ_ALWAYS_INLINE void SetData(const Data& data) { m_Data = data; }

    /// \brief Gets the root node of this mesh asset.
    EZ_NODISCARD EZ_ALWAYS_INLINE const Node& GetRootNode() const { return m_Root; }

    /// \brief Gets the root node of this mesh asset.
    EZ_ALWAYS_INLINE Node& GetRootNode() { return m_Root; }

    /// \brief Sets the root node of this mesh asset.
    /// \param [in] rootNode The root node.
    EZ_ALWAYS_INLINE void SetRootNode(const Node& rootNode) { m_Root = rootNode; }

    /// \brief Clear the mesh data.
    void Clear();

  private:
    Data m_Data;
    Node m_Root;
  };
} // namespace RAI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spMesh::Transform& transform)
{
  inout_stream << transform.m_vPosition;
  inout_stream << transform.m_vScale;
  inout_stream << transform.m_vRotation;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spMesh::Transform& ref_transform)
{
  inout_stream >> ref_transform.m_vPosition;
  inout_stream >> ref_transform.m_vScale;
  inout_stream >> ref_transform.m_vRotation;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spMesh::Entry& meshEntry)
{
  inout_stream << meshEntry.m_sName;
  inout_stream << meshEntry.m_uiBaseIndex;
  inout_stream << meshEntry.m_uiIndexCount;
  inout_stream << meshEntry.m_uiBaseVertex;
  inout_stream << meshEntry.m_uiVertexCount;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spMesh::Entry& ref_meshEntry)
{
  inout_stream >> ref_meshEntry.m_sName;
  inout_stream >> ref_meshEntry.m_uiBaseIndex;
  inout_stream >> ref_meshEntry.m_uiIndexCount;
  inout_stream >> ref_meshEntry.m_uiBaseVertex;
  inout_stream >> ref_meshEntry.m_uiVertexCount;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spMesh::Node& meshNode)
{
  inout_stream << meshNode.m_sName;
  inout_stream << meshNode.m_Transform;
  inout_stream << meshNode.m_sMaterial;
  inout_stream.WriteArray(meshNode.m_Entries).AssertSuccess();
  inout_stream.WriteArray(meshNode.m_Children).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spMesh::Node& ref_meshNode)
{
  inout_stream >> ref_meshNode.m_sName;
  inout_stream >> ref_meshNode.m_Transform;
  inout_stream >> ref_meshNode.m_sMaterial;
  inout_stream.ReadArray(ref_meshNode.m_Entries).AssertSuccess();
  inout_stream.ReadArray(ref_meshNode.m_Children).AssertSuccess();

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spMesh::Data& meshData)
{
  ezDynamicArray<ezUInt8> vertexData;
  vertexData.SetCountUninitialized(static_cast<ezUInt32>(meshopt_encodeVertexBufferBound(meshData.m_Vertices.GetCount(), sizeof(RAI::spVertex))));
  vertexData.SetCount(static_cast<ezUInt32>(meshopt_encodeVertexBuffer(vertexData.GetData(), vertexData.GetCount(), meshData.m_Vertices.GetData(), meshData.m_Vertices.GetCount(), sizeof(RAI::spVertex))));

  ezDynamicArray<ezUInt8> indexData;
  indexData.SetCountUninitialized(static_cast<ezUInt32>(meshopt_encodeIndexBufferBound(meshData.m_Indices.GetCount(), meshData.m_Vertices.GetCount())));
  indexData.SetCount(static_cast<ezUInt32>(meshopt_encodeIndexBuffer(indexData.GetData(), indexData.GetCount(), meshData.m_Indices.GetData(), meshData.m_Indices.GetCount())));

  inout_stream << meshData.m_Vertices.GetCount();
  inout_stream << meshData.m_Indices.GetCount();

  inout_stream.WriteArray(vertexData).AssertSuccess();
  inout_stream.WriteArray(indexData).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spMesh::Data& ref_meshData)
{
  ezDynamicArray<ezUInt8> vertexData;
  ezDynamicArray<ezUInt8> indexData;
  ezUInt32 uiVertexCount = 0;
  ezUInt32 uiIndexCount = 0;

  inout_stream >> uiVertexCount;
  inout_stream >> uiIndexCount;

  inout_stream.ReadArray(vertexData).AssertSuccess();
  inout_stream.ReadArray(indexData).AssertSuccess();

  ref_meshData.m_Vertices.SetCountUninitialized(uiVertexCount);
  ref_meshData.m_Indices.SetCountUninitialized(uiIndexCount);

  const ezUInt32 uiResVb = meshopt_decodeVertexBuffer(ref_meshData.m_Vertices.GetData(), uiVertexCount, sizeof(RAI::spVertex), vertexData.GetData(), vertexData.GetCount());
  const ezUInt32 uiResIb = meshopt_decodeIndexBuffer(ref_meshData.m_Indices.GetData(), uiIndexCount, indexData.GetData(), indexData.GetCount());
  EZ_ASSERT_DEV(uiResIb == 0 && uiResVb == 0, "Invalid mesh asset");

  return inout_stream;
}
