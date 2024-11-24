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

namespace RPI
{
  template <typename T>
  EZ_ALWAYS_INLINE void spMesh::Data::SetVertexStreamData(ezUInt32 uiStreamIndex, ezUInt32 uiVertexIndex, const T& pData)
  {
    EZ_ASSERT_DEV(uiStreamIndex < m_VertexStreams.GetCount(), "Invalid stream index.");
    EZ_ASSERT_DEV(uiVertexIndex < GetVertexCount(), "Invalid vertex index.");

    spMeshDataUtils::SetVertexStreamData(m_Vertices, uiVertexIndex, m_uiVertexSize, m_VertexStreams[uiStreamIndex].m_uiOffset, m_VertexStreams[uiStreamIndex].m_eFormat, pData);
  }

  EZ_ALWAYS_INLINE ezByteArrayPtr spMesh::Data::GetVertexStreamData(ezUInt32 uiStreamIndex, ezUInt32 uiVertexIndex)
  {
    EZ_ASSERT_DEV(uiStreamIndex < m_VertexStreams.GetCount(), "Invalid stream index.");
    EZ_ASSERT_DEV(uiVertexIndex < GetVertexCount(), "Invalid vertex index.");

    return ezMakeByteArrayPtr(
      spMeshDataUtils::GetVertexStreamData(m_Vertices, uiVertexIndex, m_uiVertexSize, m_VertexStreams[uiStreamIndex].m_uiOffset),
      RHI::spPixelFormatHelper::GetSizeInBytes(m_VertexStreams[uiStreamIndex].m_eFormat));
  }

  [[nodiscard]] EZ_ALWAYS_INLINE ezConstByteArrayPtr spMesh::Data::GetVertexStreamData(ezUInt32 uiStreamIndex, ezUInt32 uiVertexIndex) const
  {
    EZ_ASSERT_DEV(uiStreamIndex < m_VertexStreams.GetCount(), "Invalid stream index.");
    EZ_ASSERT_DEV(uiVertexIndex < GetVertexCount(), "Invalid vertex index.");

    return ezMakeByteArrayPtr(
      spMeshDataUtils::GetVertexStreamData(m_Vertices, uiVertexIndex, m_uiVertexSize, m_VertexStreams[uiStreamIndex].m_uiOffset),
      RHI::spPixelFormatHelper::GetSizeInBytes(m_VertexStreams[uiStreamIndex].m_eFormat));
  }

  EZ_ALWAYS_INLINE void spMesh::Data::SetVertexData(ezUInt32 uiVertexIndex, ezConstByteArrayPtr pData)
  {
    EZ_ASSERT_DEV(uiVertexIndex < GetVertexCount(), "Invalid vertex index.");
    EZ_ASSERT_DEV(pData.GetCount() == m_uiVertexSize, "Invalid vertex data size.");

    ezMemoryUtils::RawByteCopy(
      &m_Vertices[uiVertexIndex * m_uiVertexSize],
      pData.GetPtr(),
      m_uiVertexSize);
  }

  EZ_ALWAYS_INLINE ezByteArrayPtr spMesh::Data::GetVertexData(ezUInt32 uiVertexIndex)
  {
    EZ_ASSERT_DEV(uiVertexIndex < GetVertexCount(), "Invalid vertex index.");
    return ezMakeByteArrayPtr(&m_Vertices[uiVertexIndex * m_uiVertexSize], m_uiVertexSize);
  }

  [[nodiscard]] EZ_ALWAYS_INLINE ezConstByteArrayPtr spMesh::Data::GetVertexData(ezUInt32 uiVertexIndex) const
  {
    EZ_ASSERT_DEV(uiVertexIndex < GetVertexCount(), "Invalid vertex index.");
    return ezMakeByteArrayPtr(&m_Vertices[uiVertexIndex * m_uiVertexSize], m_uiVertexSize);
  }

  EZ_ALWAYS_INLINE void spMesh::Data::SetPrimitiveIndices(ezUInt32 uiPrimitiveIndex, ezUInt32 uiVertex0, ezUInt32 uiVertex1, ezUInt32 uiVertex2)
  {
    if (m_eTopology == RHI::spPrimitiveTopology::Points)
    {
      auto* pIndices = reinterpret_cast<ezUInt16*>(m_Indices.GetData() + (uiPrimitiveIndex * sizeof(ezUInt16)));
      pIndices[0] = uiVertex0;
    }
    else if (m_eTopology == RHI::spPrimitiveTopology::Lines)
    {
      auto* pIndices = reinterpret_cast<ezUInt16*>(m_Indices.GetData() + (uiPrimitiveIndex * sizeof(ezUInt16) * 2));
      pIndices[0] = uiVertex0;
      pIndices[1] = uiVertex1;
    }
    else if (m_eTopology == RHI::spPrimitiveTopology::Triangles)
    {
      auto* pIndices = reinterpret_cast<ezUInt16*>(m_Indices.GetData() + (uiPrimitiveIndex * sizeof(ezUInt16) * 3));
      pIndices[0] = uiVertex0;
      pIndices[1] = uiVertex1;
      pIndices[2] = uiVertex2;
    }
  }

  [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 spMesh::Data::GetStreamOffset(ezUInt32 uiStreamIndex) const
  {
    EZ_ASSERT_DEV(uiStreamIndex < m_VertexStreams.GetCount(), "Invalid stream index.");
    return m_VertexStreams[uiStreamIndex].m_uiOffset;
  }

  [[nodiscard]] EZ_ALWAYS_INLINE RHI::spInputElementFormat::Enum spMesh::Data::GetStreamFormat(ezUInt32 uiStreamIndex) const
  {
    EZ_ASSERT_DEV(uiStreamIndex < m_VertexStreams.GetCount(), "Invalid stream index.");
    return m_VertexStreams[uiStreamIndex].m_eFormat;
  }

  [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 spMesh::Data::GetStreamSize(ezUInt32 uiStreamIndex) const
  {
    EZ_ASSERT_DEV(uiStreamIndex < m_VertexStreams.GetCount(), "Invalid stream index.");
    return RHI::spPixelFormatHelper::GetSizeInBytes(m_VertexStreams[uiStreamIndex].m_eFormat);
  }

  EZ_ALWAYS_INLINE void spMesh::SetData(const RPI::spMesh::Data& data)
  {
    Clear();
    m_Data = data;
  }

  template <typename T>
  EZ_ALWAYS_INLINE void spMeshDataUtils::SetVertexStreamData(ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper>& ref_VertexData, ezUInt32 uiVertexIndex, ezUInt32 uiVertexSize, ezUInt32 uiStreamOffset, RHI::spInputElementFormat::Enum eFormat, const T& vertexData)
  {
    reinterpret_cast<T&>(ref_VertexData[uiVertexIndex * uiVertexSize + uiStreamOffset]) = vertexData;
  }

  EZ_ALWAYS_INLINE ezUInt8* spMeshDataUtils::GetVertexStreamData(ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper>& in_VertexData, ezUInt32 uiVertexIndex, ezUInt32 uiVertexSize, ezUInt32 uiStreamOffset)
  {
    return &in_VertexData[uiVertexIndex * uiVertexSize + uiStreamOffset];
  }

  EZ_ALWAYS_INLINE const ezUInt8* spMeshDataUtils::GetVertexStreamData(const ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper>& vertexData, ezUInt32 uiVertexIndex, ezUInt32 uiVertexSize, ezUInt32 uiStreamOffset)
  {
    return &vertexData[uiVertexIndex * uiVertexSize + uiStreamOffset];
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::EncodeNormal(const ezVec3& vNormal, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    // Normals are stored in unsigned formats, so we need to map from -1..1 range to 0..1
    return EncodeVec3(vNormal * 0.5f + ezVec3(0.5f), pDest, eFormat);
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::EncodeTangent(const ezVec3& vTangent, float fMagnitude, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    // Tangents are stored in unsigned formats, so we need to map from -1..1 range to 0..1
    return EncodeVec4((vTangent * 0.5f + ezVec3(0.5f)).GetAsVec4(fMagnitude), pDest, eFormat);
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::EncodeTexCoord(const ezVec2& vTexCoord, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    return EncodeVec2(vTexCoord, pDest, eFormat);
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::EncodeBoneWeights(const ezVec4& vWeights, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    return EncodeVec4(vWeights, pDest, eFormat);
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::EncodeColor(const ezColor& vColor, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    return EncodeVec4(vColor.GetAsVec4(), pDest, eFormat);
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::DecodeNormal(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec3& ref_vDestNormal)
  {
    ezVec3 vNormal;
    EZ_SUCCEED_OR_RETURN(DecodeVec3(pSource, eFormat, vNormal));
    ref_vDestNormal = vNormal * 2.0f - ezVec3(1.0f);
    return EZ_SUCCESS;
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::DecodeTangent(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec3& ref_vDestTangent, float& ref_fMagnitude)
  {
    ezVec4 vTangent;
    EZ_SUCCEED_OR_RETURN(DecodeVec4(pSource, eFormat, vTangent));
    ref_vDestTangent = vTangent.GetAsVec3() * 2.0f - ezVec3(1.0f);
    ref_fMagnitude = vTangent.w;
    return EZ_SUCCESS;
  }

  EZ_ALWAYS_INLINE ezResult spMeshDataUtils::DecodeTexCoord(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec2& ref_vDestTexCoord)
  {
    return DecodeVec2(pSource, eFormat, ref_vDestTexCoord);
  }
} // namespace RPI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMesh::VertexStream& stream)
{
  inout_stream << stream.m_eFormat;
  inout_stream << stream.m_uiOffset;
  inout_stream << stream.m_eSemantic;
  inout_stream << stream.m_uiSemanticIndex;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMesh::VertexStream& ref_stream)
{
  inout_stream >> ref_stream.m_eFormat;
  inout_stream >> ref_stream.m_uiOffset;
  inout_stream >> ref_stream.m_eSemantic;
  inout_stream >> ref_stream.m_uiSemanticIndex;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMesh::Transform& transform)
{
  inout_stream << transform.m_vPosition;
  inout_stream << transform.m_vScale;
  inout_stream << transform.m_vRotation;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMesh::Transform& ref_transform)
{
  inout_stream >> ref_transform.m_vPosition;
  inout_stream >> ref_transform.m_vScale;
  inout_stream >> ref_transform.m_vRotation;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMesh::Entry& meshEntry)
{
  inout_stream << meshEntry.m_sName;
  inout_stream << meshEntry.m_uiBaseIndex;
  inout_stream << meshEntry.m_uiIndexCount;
  inout_stream << meshEntry.m_uiBaseVertex;
  inout_stream << meshEntry.m_uiVertexCount;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMesh::Entry& ref_meshEntry)
{
  inout_stream >> ref_meshEntry.m_sName;
  inout_stream >> ref_meshEntry.m_uiBaseIndex;
  inout_stream >> ref_meshEntry.m_uiIndexCount;
  inout_stream >> ref_meshEntry.m_uiBaseVertex;
  inout_stream >> ref_meshEntry.m_uiVertexCount;

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMesh::Node& meshNode)
{
  inout_stream << meshNode.m_sName;
  inout_stream << meshNode.m_Transform;
  inout_stream << meshNode.m_sMaterial;
  inout_stream.WriteArray(meshNode.m_Entries).AssertSuccess();
  inout_stream.WriteArray(meshNode.m_Children).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMesh::Node& ref_meshNode)
{
  inout_stream >> ref_meshNode.m_sName;
  inout_stream >> ref_meshNode.m_Transform;
  inout_stream >> ref_meshNode.m_sMaterial;
  inout_stream.ReadArray(ref_meshNode.m_Entries).AssertSuccess();
  inout_stream.ReadArray(ref_meshNode.m_Children).AssertSuccess();

  return inout_stream;
}
