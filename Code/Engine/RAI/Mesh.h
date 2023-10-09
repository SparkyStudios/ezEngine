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
  /// \brief A mesh asset.
  ///
  /// Stores all needed data to render a mesh.
  class SP_RAI_DLL spMesh
  {
    friend class spMeshResource;
    friend class spMeshResourceDescriptor;
    friend class spMeshDataBuilder;

#pragma region RAI Resource

  public:
    /// \brief Describes a single vertex stream in the raw vertices data.
    ///
    /// Each vertex stream contains a single vertex attribute. All the streams
    /// are interleaved in the raw vertices data. The offset of each stream is
    /// specified in the stream descriptor by the VertexStream::m_uiOffset field.
    struct VertexStream : ezHashableStruct<VertexStream>
    {
      EZ_DECLARE_POD_TYPE();

      /// \brief The offset of the vertex stream in a single attributes array.
      ezUInt32 m_uiOffset{0};

      /// \brief The semantic of the vertex stream.
      ezEnum<RHI::spInputElementLocationSemantic> m_eSemantic;

      /// \brief The semantic index of the vertex stream.
      ezUInt32 m_uiSemanticIndex{0};

      /// \brief The format of the vertex stream.
      ezEnum<RHI::spInputElementFormat> m_eFormat;

      EZ_ALWAYS_INLINE bool operator<(const VertexStream& rhs) const
      {
        return m_eSemantic != rhs.m_eSemantic ? m_eSemantic < rhs.m_eSemantic : m_uiSemanticIndex < rhs.m_uiSemanticIndex;
      }

      EZ_ALWAYS_INLINE bool operator==(const VertexStream& rhs) const
      {
        return m_eSemantic == rhs.m_eSemantic && m_uiSemanticIndex == rhs.m_uiSemanticIndex;
      }
    };

    /// \brief The mesh data. Stores all the vertices and indices.
    struct Data
    {
      /// \brief The mesh vertices raw data.
      ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper> m_Vertices;

      /// \brief The mesh indices raw data.
      ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper> m_Indices;

      ezHybridArray<VertexStream, 24> m_VertexStreams;

      /// \brief The mesh topology.
      ezEnum<RHI::spPrimitiveTopology> m_eTopology;

      /// \brief The size of a single vertex in bytes.
      ezUInt32 m_uiVertexSize{0};

      EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetVertexCount() const { return m_Vertices.GetCount() / m_uiVertexSize; }

      EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetVertexBufferSize() const { return m_Vertices.GetCount(); }

      EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetIndexCount() const { return m_Indices.GetCount() / sizeof(ezUInt16); }

      EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetIndexBufferSize() const { return m_Indices.GetCount(); }

      template <typename T>
      EZ_ALWAYS_INLINE void SetVertexStreamData(ezUInt32 uiStreamIndex, ezUInt32 uiVertexIndex, const T& data);

      /// \brief Gets the vertex data at the given index for the given stream.
      /// \param [in] uiStreamIndex The index of the vertex stream.
      /// \param [in] uiVertexIndex The index of the vertex.
      /// \return The vertex stream data.
      ezByteArrayPtr GetVertexStreamData(ezUInt32 uiStreamIndex, ezUInt32 uiVertexIndex);

      /// \brief Gets the vertex data at the given index for the given stream.
      /// \param [in] uiStreamIndex The index of the vertex stream.
      /// \param [in] uiVertexIndex The index of the vertex.
      /// \return The vertex stream data.
      EZ_NODISCARD ezConstByteArrayPtr GetVertexStreamData(ezUInt32 uiStreamIndex, ezUInt32 uiVertexIndex) const;

      /// \brief Copies the vertex data from the given source buffer.
      /// \param [in] uiVertexIndex The index of the destination vertex.
      /// \param [in] pData The source buffer.
      void SetVertexData(ezUInt32 uiVertexIndex, ezConstByteArrayPtr pData);

      /// \brief Gets the data of the vertex at the given index.
      /// \param [in] uiVertexIndex The index of the vertex.
      /// \return A pointer to the data of the vertex at the given index.
      ezByteArrayPtr GetVertexData(ezUInt32 uiVertexIndex);

      /// \brief Gets the data of the vertex at the given index.
      /// \param [in] uiVertexIndex The index of the vertex.
      /// \return A pointer to the data of the vertex at the given index.
      EZ_NODISCARD ezConstByteArrayPtr GetVertexData(ezUInt32 uiVertexIndex) const;

      /// \brief Sets the indices of the primitive at the given index.
      /// \param [in] uiPrimitiveIndex The index of the primitive.
      /// \param [in] uiVertex0 The index of the first vertex.
      /// \param [in] uiVertex1 The index of the second vertex. Only used for lines and triangles.
      /// \param [in] uiVertex2 The index of the third vertex. Only used for triangles.
      void SetPrimitiveIndices(ezUInt32 uiPrimitiveIndex, ezUInt32 uiVertex0, ezUInt32 uiVertex1 = 0, ezUInt32 uiVertex2 = 0);

      /// \brief Gets the offset of the stream at the given index.
      /// \param [in] uiStreamIndex The index of the stream.
      /// \return The offset of the stream at the given index.
      EZ_NODISCARD ezUInt32 GetStreamOffset(ezUInt32 uiStreamIndex) const;

      /// \brief Gets the format of the stream at the given index.
      /// \param [in] uiStreamIndex The index of the stream.
      /// \return The format of the stream at the given index.
      EZ_NODISCARD RHI::spInputElementFormat::Enum GetStreamFormat(ezUInt32 uiStreamIndex) const;
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
    void SetData(const Data& data);

    /// \brief Gets the root node of this mesh asset.
    EZ_NODISCARD EZ_ALWAYS_INLINE const Node& GetRootNode() const { return m_Root; }

    /// \brief Gets the root node of this mesh asset.
    EZ_ALWAYS_INLINE Node& GetRootNode() { return m_Root; }

    /// \brief Sets the root node of this mesh asset.
    /// \param [in] rootNode The root node.
    EZ_ALWAYS_INLINE void SetRootNode(const Node& rootNode) { m_Root = rootNode; }

    /// \brief Clear the mesh data.
    void Clear();

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetVertexSize() const { return m_Data.m_uiVertexSize; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetVertexCount() const { return m_Data.GetVertexCount(); }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetVertexBufferSize() const { return m_Data.GetVertexBufferSize(); }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetIndexCount() const { return m_Data.GetIndexCount(); }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetIndexBufferSize() const { return m_Data.GetIndexBufferSize(); }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezConstByteArrayPtr GetVertexBuffer() const { return m_Data.m_Vertices; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezConstByteArrayPtr GetIndexBuffer() const { return m_Data.m_Indices; }

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
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spBuffer> GetRHIVertexBuffer() const { return m_pRHIVertexBuffer; }

    /// \brief Gets the RHI buffer resource for the index buffer.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spBuffer> GetRHIIndexBuffer() const { return m_pRHIIndexBuffer; }

    /// \brief Gets the RHI buffer resource for the indirect buffer (draw commands).
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spBuffer> GetRHIIndirectBuffer() const { return m_pRHIIndirectBuffer; }

    void GetRHIInputLayoutDescription(RHI::spInputLayoutDescription& out_InputLayoutDescription) const;

    /// \brief Generate draw commands.
    /// \param [out] out_DrawCommands The generated array of draw commands.
    void GetDrawCommands(ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper>& out_DrawCommands) const;

  private:
    ezSharedPtr<RHI::spBuffer> m_pRHIVertexBuffer{nullptr};
    ezSharedPtr<RHI::spBuffer> m_pRHIIndexBuffer{nullptr};
    ezSharedPtr<RHI::spBuffer> m_pRHIIndirectBuffer{nullptr};

#pragma endregion
  };

  /// \brief A mesh data builder.
  class SP_RAI_DLL spMeshDataBuilder
  {
  public:
    /// \brief Adds a new vertex stream in the mesh.
    /// \param [in] stream The vertex stream to add.
    /// \return The index of the vertex stream in streams array.
    ezUInt32 AddVertexStream(const spMesh::VertexStream& stream);

    /// \brief Adds a new vertex stream in the mesh.
    /// \param [in] eSemantic The semantic of the vertex stream.
    /// \param [in] uiSemanticIndex The semantic index of the vertex stream.
    /// \param [in] eFormat The format of the vertex stream.
    /// \return The index of the vertex stream in the streams array.
    ezUInt32 AddVertexStream(const ezEnum<RHI::spInputElementLocationSemantic>& eSemantic, ezUInt32 uiSemanticIndex, const ezEnum<RHI::spInputElementFormat>& eFormat);

    /// \brief Allocates the required memory for the mesh data with the configured streams.
    /// \param [out] ref_MeshData The mesh data instance in which the data will be stored.
    /// \param [in] uiMaxVertexCount The maximum number of vertices.
    /// \param [in] eTopology The primitive topology.
    /// \param [in] uiNumPrimitives The number of primitives.
    /// \param [in] bZeroFill If true, the vertex data will be zero filled.
    void AllocateMeshData(spMesh::Data& ref_MeshData, ezUInt32 uiMaxVertexCount, const ezEnum<RHI::spPrimitiveTopology>& eTopology, ezUInt32 uiNumPrimitives, bool bZeroFill = false);

  private:
    ezDynamicArray<spMesh::VertexStream> m_VertexStreams;

    /// \brief The size of a single vertex in bytes.
    ezUInt32 m_uiVertexSize{0};
  };

  class SP_RAI_DLL spMeshDataUtils
  {
  public:
    template <typename T>
    static void SetVertexStreamData(ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper>& ref_VertexData, ezUInt32 uiVertexIndex, ezUInt32 uiVertexSize, ezUInt32 uiStreamOffset, RHI::spInputElementFormat::Enum eFormat, const T& vertexData);

    static ezUInt8* GetVertexStreamData(ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper>& in_VertexData, ezUInt32 uiVertexIndex, ezUInt32 uiVertexSize, ezUInt32 uiStreamOffset);
    static const ezUInt8* GetVertexStreamData(const ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper>& vertexData, ezUInt32 uiVertexIndex, ezUInt32 uiVertexSize, ezUInt32 uiStreamOffset);

    static ezResult EncodeNormal(const ezVec3& vNormal, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);
    static ezResult EncodeTangent(const ezVec3& vTangent, float fMagnitude, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);
    static ezResult EncodeTexCoord(const ezVec2& vTexCoord, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);
    static ezResult EncodeBoneWeights(const ezVec4& vWeights, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);
    static ezResult EncodeColor(const ezColor& vColor, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);

    static ezResult DecodeNormal(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec3& ref_vDestNormal);
    static ezResult DecodeTangent(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec3& ref_vDestTangent, float& ref_fMagnitude);
    static ezResult DecodeTexCoord(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec2& ref_vDestTexCoord);

    static ezResult EncodeFloat(const float& fSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);
    static ezResult EncodeVec2(const ezVec2& vSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);
    static ezResult EncodeVec3(const ezVec3& vSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);
    static ezResult EncodeVec4(const ezVec4& vSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat);

    static ezResult DecodeFloat(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, float& ref_fDest);
    static ezResult DecodeVec2(const ezConstByteArrayPtr& psource, RHI::spInputElementFormat::Enum eFormat, ezVec2& ref_vDest);
    static ezResult DecodeVec3(const ezConstByteArrayPtr& psource, RHI::spInputElementFormat::Enum eFormat, ezVec3& ref_vDest);
    static ezResult DecodeVec4(const ezConstByteArrayPtr& psource, RHI::spInputElementFormat::Enum eFormat, ezVec4& ref_vDest);
  };
} // namespace RAI

#include <RAI/Implementation/Mesh_inl.h>
