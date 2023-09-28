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

#include <RAI/RAIPCH.h>

#include <RAI/Mesh.h>

#include <RHI/Device.h>

#include <meshoptimizer.h>

namespace RAI
{
  static void GetDrawCommandsInternal(ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper>& out_DrawCommands, const spMesh::Node& node)
  {
    for (const auto& entry : node.m_Entries)
    {
      RHI::spDrawIndexedIndirectCommand cmd;
      cmd.m_uiCount = entry.m_uiIndexCount;
      cmd.m_uiInstanceCount = 1;
      cmd.m_uiFirstIndex = entry.m_uiBaseIndex;
      cmd.m_uiBaseVertex = entry.m_uiBaseVertex;
      cmd.m_uiBaseInstance = 0;

      out_DrawCommands.PushBack(cmd);
    }

    for (const auto& child : node.m_Children)
      GetDrawCommandsInternal(out_DrawCommands, child);
  }

  spMesh::~spMesh() noexcept
  {
    Clear();
  }

  void spMesh::Clear()
  {
    m_Data.m_Indices.Clear();
    m_Data.m_Vertices.Clear();

    m_Root.m_Children.Clear();
    m_Root.m_Entries.Clear();
    m_Root.m_sName.Clear();
    m_Root.m_Transform = {};

    m_pRHIVertexBuffer.Clear();
    m_pRHIIndexBuffer.Clear();
    m_pRHIIndirectBuffer.Clear();
  }

  void spMesh::GetDrawCommands(ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper>& out_DrawCommands) const
  {
    GetDrawCommandsInternal(out_DrawCommands, m_Root);
  }

  void spMesh::CreateRHIVertexBuffer()
  {
    if (m_pRHIVertexBuffer != nullptr)
      return;

    EZ_ASSERT_DEV(!m_Data.m_Vertices.IsEmpty(), "Mesh has no vertices.");
    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    ezUInt32 uiVertexCount = m_Data.m_Vertices.GetCount();
    m_pRHIVertexBuffer = pDevice->GetResourceFactory()->CreateBuffer(RHI::spBufferDescription(uiVertexCount * sizeof(spVertex), RHI::spBufferUsage::VertexBuffer));

    pDevice->UpdateBuffer<spVertex>(m_pRHIVertexBuffer, 0, m_Data.m_Vertices.GetArrayPtr());

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.Format("{0}__VertexBuffer", m_Root.m_sName);
    m_pRHIVertexBuffer->SetDebugName(sb);
#endif
  }

  void spMesh::CreateRHIIndexBuffer()
  {
    if (m_pRHIIndexBuffer != nullptr)
      return;

    EZ_ASSERT_DEV(!m_Data.m_Indices.IsEmpty(), "Mesh has no indices.");
    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    ezUInt32 uiIndexCount = m_Data.m_Indices.GetCount();
    m_pRHIIndexBuffer = pDevice->GetResourceFactory()->CreateBuffer(RHI::spBufferDescription(uiIndexCount * sizeof(ezUInt32), RHI::spBufferUsage::IndexBuffer));

    pDevice->UpdateBuffer<ezUInt16>(m_pRHIIndexBuffer, 0, m_Data.m_Indices.GetArrayPtr());

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.Format("{0}__IndexBuffer", m_Root.m_sName);
    m_pRHIIndexBuffer->SetDebugName(sb);
#endif
  }

  void spMesh::CreateRHIIndirectBuffer()
  {
    if (m_pRHIIndirectBuffer != nullptr)
      return;

    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper> drawCommands;
    GetDrawCommands(drawCommands);

    m_pRHIIndirectBuffer = pDevice->GetResourceFactory()->CreateBuffer(RHI::spBufferDescription(sizeof(RHI::spDrawIndexedIndirectCommand) * drawCommands.GetCount(), RHI::spBufferUsage::IndirectBuffer));

    pDevice->UpdateBuffer<RHI::spDrawIndexedIndirectCommand>(m_pRHIIndirectBuffer, 0, drawCommands.GetArrayPtr());

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.Format("{0}__IndirectBuffer", m_Root.m_sName);
    m_pRHIIndirectBuffer->SetDebugName(sb);
#endif
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
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Mesh);
