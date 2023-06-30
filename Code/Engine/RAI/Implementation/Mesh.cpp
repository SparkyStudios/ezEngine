#include <RAI/RAIPCH.h>

#include <RAI/Mesh.h>

#include <meshoptimizer.h>

using namespace RAI;

void spMesh::Clear()
{
  m_Data.m_Indices.Clear();
  m_Data.m_Vertices.Clear();

  m_Root.m_Children.Clear();
  m_Root.m_Entries.Clear();
  m_Root.m_sName.Clear();
  m_Root.m_Transform = {};
}

void spMesh::ReadData(ezStreamReader& inout_stream)
{
  ezDynamicArray<ezUInt8> vertexData;
  ezDynamicArray<ezUInt8> indexData;
  ezUInt32 uiVertexCount = 0;
  ezUInt32 uiIndexCount = 0;

  inout_stream >> uiVertexCount;
  inout_stream >> uiIndexCount;

  inout_stream.ReadArray(vertexData).AssertSuccess();
  inout_stream.ReadArray(indexData).AssertSuccess();

  m_Data.m_Vertices.SetCountUninitialized(uiVertexCount);
  m_Data.m_Indices.SetCountUninitialized(uiIndexCount);

  const ezUInt32 uiResVb = meshopt_decodeVertexBuffer(m_Data.m_Vertices.GetData(), uiVertexCount, sizeof(RAI::spVertex), vertexData.GetData(), vertexData.GetCount());
  const ezUInt32 uiResIb = meshopt_decodeIndexBuffer(m_Data.m_Indices.GetData(), uiIndexCount, indexData.GetData(), indexData.GetCount());
  EZ_ASSERT_DEV(uiResIb == 0 && uiResVb == 0, "Invalid mesh asset");
}

void spMesh::WriteData(ezStreamWriter& inout_stream) const
{
  ezDynamicArray<ezUInt8> vertexData;
  vertexData.SetCountUninitialized(static_cast<ezUInt32>(meshopt_encodeVertexBufferBound(m_Data.m_Vertices.GetCount(), sizeof(RAI::spVertex))));
  vertexData.SetCount(static_cast<ezUInt32>(meshopt_encodeVertexBuffer(vertexData.GetData(), vertexData.GetCount(), m_Data.m_Vertices.GetData(), m_Data.m_Vertices.GetCount(), sizeof(RAI::spVertex))));

  ezDynamicArray<ezUInt8> indexData;
  indexData.SetCountUninitialized(static_cast<ezUInt32>(meshopt_encodeIndexBufferBound(m_Data.m_Indices.GetCount(), m_Data.m_Vertices.GetCount())));
  indexData.SetCount(static_cast<ezUInt32>(meshopt_encodeIndexBuffer(indexData.GetData(), indexData.GetCount(), m_Data.m_Indices.GetData(), m_Data.m_Indices.GetCount())));

  inout_stream << m_Data.m_Vertices.GetCount();
  inout_stream << m_Data.m_Indices.GetCount();

  inout_stream.WriteArray(vertexData).AssertSuccess();
  inout_stream.WriteArray(indexData).AssertSuccess();
}
