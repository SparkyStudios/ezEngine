// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#include <RAI/RAIDLL.h>

#include <RAI/Core.h>

#include <RHI/CommandList.h>

namespace RAI
{
  /// \brief A mesh asset. Stores all needed data to render a mesh.
  class SP_RAI_DLL spMesh
  {
    friend class spMeshResource;
    friend class spMeshResourceDescriptor;

#pragma region RAI Resource

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

    ~spMesh() noexcept;

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

    /// \brief Generate draw commands.
    /// \param [out] out_DrawCommands The generated array of draw commands.
    void GetDrawCommands(ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper>& out_DrawCommands) const;

  private:
    void ReadData(ezStreamReader& inout_stream);
    void WriteData(ezStreamWriter& inout_stream) const;

    Data m_Data;
    Node m_Root;

#pragma endregion

#pragma region RHI Resources

  public:
    /// \brief Creates a RHI buffer resource for the vertex
    /// buffer of this mesh.
    void CreateRHIVertexBuffer();

    /// \brief Creates a RHI buffer resource for the index
    /// buffer of this mesh.
    void CreateRHIIndexBuffer();

    /// \brief Creates a RHI buffer resource for the draw commands
    /// of this mesh.
    void CreateRHIIndirectBuffer();

    /// \brief Gets the RHI buffer resource for the vertex buffer.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spBuffer> GetRHIVertexBuffer() const { return m_RHIVertexBuffer; }

    /// \brief Gets the RHI buffer resource for the index buffer.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spBuffer> GetRHIIndexBuffer() const { return m_RHIIndexBuffer; }

    /// \brief Gets the RHI buffer resource for the indirect buffer (draw commands).
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spBuffer> GetRHIIndirectBuffer() const { return m_RHIIndirectBuffer; }

  private:
    ezSharedPtr<RHI::spBuffer> m_RHIVertexBuffer{nullptr};
    ezSharedPtr<RHI::spBuffer> m_RHIIndexBuffer{nullptr};
    ezSharedPtr<RHI::spBuffer> m_RHIIndirectBuffer{nullptr};

#pragma endregion
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
