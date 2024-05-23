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

#include <Foundation/Math/Float16.h>

#include <meshoptimizer.h>

namespace RAI
{
  template <ezUInt32 Bits>
  EZ_ALWAYS_INLINE ezUInt32 ColorFloatToUNorm(float fValue)
  {
    // Implemented according to
    // https://docs.microsoft.com/en-us/windows/desktop/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
    if (ezMath::IsNaN(fValue))
      return 0;

    float fMaxValue = ((1 << Bits) - 1);
    return static_cast<ezUInt32>(ezMath::Saturate(fValue) * fMaxValue + 0.5f);
  }

  template <ezUInt32 Bits>
  constexpr EZ_FORCE_INLINE float ColorUNormToFloat(ezUInt32 value)
  {
    // Implemented according to
    // https://docs.microsoft.com/en-us/windows/desktop/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
    ezUInt32 uiMaxValue = ((1 << Bits) - 1);
    float fMaxValue = ((1 << Bits) - 1);
    return (value & uiMaxValue) * (1.0f / fMaxValue);
  }

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
    m_Data.m_VertexStreams.Clear();
    m_Data.m_uiVertexSize = 0;

    m_Root.m_Children.Clear();
    m_Root.m_Entries.Clear();
    m_Root.m_sName.Clear();
    m_Root.m_Transform = {};

    m_pRHIVertexBuffer.Clear();
    m_pRHIIndexBuffer.Clear();
    m_pRHIIndirectBuffer.Clear();
  }

  void spMesh::ComputeBounds()
  {
    m_Bounds = ezBoundingBoxSphere::MakeInvalid();

    for (ezUInt32 i = 0; i < m_Data.m_VertexStreams.GetCount(); ++i)
    {
      if (m_Data.m_VertexStreams[i].m_eSemantic == RHI::spInputElementLocationSemantic::Position)
      {
        EZ_ASSERT_DEBUG(m_Data.m_VertexStreams[i].m_eFormat == RHI::spInputElementFormat::Float3, "Position format is not usable");

        const ezUInt32 offset = m_Data.m_VertexStreams[i].m_uiOffset;

        if (!m_Data.m_Vertices.IsEmpty())
        {
          m_Bounds = ezBoundingBoxSphere::MakeFromPoints(reinterpret_cast<const ezVec3*>(&m_Data.m_Vertices[offset]), m_Data.GetVertexCount(), m_Data.m_uiVertexSize);
        }

        break;
      }
    }
  }

  void spMesh::ReadData(ezStreamReader& inout_stream)
  {
    ezDynamicArray<ezUInt8> vertexData;
    ezDynamicArray<ezUInt8> indexData;

    ezUInt32 uiVertexCount = 0;
    ezUInt32 uiIndexCount = 0;

    inout_stream >> uiVertexCount;
    inout_stream >> uiIndexCount;

    inout_stream >> m_Data.m_eTopology;
    inout_stream >> m_Data.m_uiVertexSize;
    inout_stream.ReadArray(m_Data.m_VertexStreams).AssertSuccess();

    inout_stream.ReadArray(vertexData).AssertSuccess();
    inout_stream.ReadArray(indexData).AssertSuccess();

    m_Data.m_Vertices.SetCountUninitialized(uiVertexCount * m_Data.m_uiVertexSize);
    m_Data.m_Indices.SetCountUninitialized(uiIndexCount * sizeof(ezUInt16));

    const ezUInt32 uiResVb = meshopt_decodeVertexBuffer(m_Data.m_Vertices.GetData(), uiVertexCount, m_Data.m_uiVertexSize, vertexData.GetData(), vertexData.GetCount());
    const ezUInt32 uiResIb = meshopt_decodeIndexBuffer(reinterpret_cast<ezUInt16*>(m_Data.m_Indices.GetData()), uiIndexCount, indexData.GetData(), indexData.GetCount());
    EZ_ASSERT_DEV(uiResIb == 0 && uiResVb == 0, "Invalid mesh asset");

    inout_stream >> m_Bounds;
    if (!m_Bounds.IsValid())
      ComputeBounds();
  }

  void spMesh::WriteData(ezStreamWriter& inout_stream) const
  {
    ezDynamicArray<ezUInt8> vertexData;
    vertexData.SetCountUninitialized(static_cast<ezUInt32>(meshopt_encodeVertexBufferBound(GetVertexCount(), GetVertexSize())));
    vertexData.SetCount(static_cast<ezUInt32>(meshopt_encodeVertexBuffer(vertexData.GetData(), vertexData.GetCount(), m_Data.m_Vertices.GetData(), GetVertexCount(), GetVertexSize())));

    ezDynamicArray<ezUInt8> indexData;
    indexData.SetCountUninitialized(static_cast<ezUInt32>(meshopt_encodeIndexBufferBound(GetIndexCount(), GetVertexCount())));
    indexData.SetCount(static_cast<ezUInt32>(meshopt_encodeIndexBuffer(indexData.GetData(), indexData.GetCount(), reinterpret_cast<const ezUInt16*>(m_Data.m_Indices.GetData()), GetIndexCount())));

    inout_stream << GetVertexCount();
    inout_stream << GetIndexCount();

    inout_stream << m_Data.m_eTopology;
    inout_stream << m_Data.m_uiVertexSize;
    inout_stream.WriteArray(m_Data.m_VertexStreams).AssertSuccess();

    inout_stream.WriteArray(vertexData).AssertSuccess();
    inout_stream.WriteArray(indexData).AssertSuccess();

    inout_stream << m_Bounds;
  }

  void spMesh::CreateRHIVertexBuffer()
  {
    if (m_pRHIVertexBuffer != nullptr)
      return;

    EZ_ASSERT_DEV(!m_Data.m_Vertices.IsEmpty(), "Mesh has no vertices.");
    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    m_pRHIVertexBuffer = pDevice->GetResourceFactory()->CreateBuffer(RHI::spBufferDescription(GetVertexBufferSize(), RHI::spBufferUsage::VertexBuffer | RHI::spBufferUsage::Dynamic));

    pDevice->UpdateBuffer(m_pRHIVertexBuffer, 0, m_Data.m_Vertices.GetArrayPtr());

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.SetFormat("{0}__VertexBuffer", m_Root.m_sName);
    m_pRHIVertexBuffer->SetDebugName(sb);
#endif
  }

  void spMesh::CreateRHIIndexBuffer()
  {
    if (m_pRHIIndexBuffer != nullptr)
      return;

    EZ_ASSERT_DEV(!m_Data.m_Indices.IsEmpty(), "Mesh has no indices.");
    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    m_pRHIIndexBuffer = pDevice->GetResourceFactory()->CreateBuffer(RHI::spBufferDescription(GetIndexBufferSize(), RHI::spBufferUsage::IndexBuffer | RHI::spBufferUsage::Dynamic));

    pDevice->UpdateBuffer(m_pRHIIndexBuffer, 0, m_Data.m_Indices.GetArrayPtr());

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.SetFormat("{0}__IndexBuffer", m_Root.m_sName);
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

    m_pRHIIndirectBuffer = pDevice->GetResourceFactory()->CreateBuffer(RHI::spBufferDescription(pDevice->GetIndexedIndirectCommandSize() * drawCommands.GetCount(), RHI::spBufferUsage::IndirectBuffer));

    pDevice->UpdateIndexedIndirectBuffer(m_pRHIIndirectBuffer, drawCommands.GetArrayPtr());

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.SetFormat("{0}__IndirectBuffer", m_Root.m_sName);
    m_pRHIIndirectBuffer->SetDebugName(sb);
#endif
  }

  void spMesh::CreateRHIInputLayout()
  {
    if (m_pRHIInputLayout != nullptr)
      return;

    const auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    RHI::spInputLayoutDescription desc;

    desc.m_Elements.Clear();
    desc.m_uiStride = m_Data.m_uiVertexSize;
    desc.m_uiInstanceStepRate = 0;

    desc.m_Elements.SetCount(m_Data.m_VertexStreams.GetCount());
    for (ezUInt32 i = 0, l = m_Data.m_VertexStreams.GetCount(); i < l; ++i)
    {
      auto& element = desc.m_Elements[i];
      element.m_eSemantic = m_Data.m_VertexStreams[i].m_eSemantic;
      element.m_eFormat = m_Data.m_VertexStreams[i].m_eFormat;
      element.m_uiOffset = m_Data.m_VertexStreams[i].m_uiOffset;
    }

    m_pRHIInputLayout = pDevice->GetResourceFactory()->CreateInputLayout(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezStringBuilder sb;
    sb.SetFormat("{0}__InputLayout", m_Root.m_sName);
    m_pRHIInputLayout->SetDebugName(sb);
#endif
  }

  void spMesh::GetDrawCommands(ezDynamicArray<RHI::spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper>& out_DrawCommands) const
  {
    GetDrawCommandsInternal(out_DrawCommands, m_Root);
  }

  ezUInt32 spMeshDataBuilder::AddVertexStream(const RAI::spMesh::VertexStream& stream)
  {
    return AddVertexStream(stream.m_eSemantic, stream.m_uiSemanticIndex, stream.m_eFormat);
  }

  ezUInt32 spMeshDataBuilder::AddVertexStream(const ezEnum<RHI::spInputElementLocationSemantic>& eSemantic, ezUInt32 uiSemanticIndex, const ezEnum<RHI::spInputElementFormat>& eFormat)
  {
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    for (ezUInt32 i = 0; i < m_VertexStreams.GetCount(); ++i)
      if (m_VertexStreams[i].m_eSemantic == eSemantic)
        EZ_ASSERT_DEV(m_VertexStreams[i].m_uiSemanticIndex != uiSemanticIndex, "The given semantic {0} is already used by a previous stream", eSemantic);
#endif

    ezUInt32 uiOffset = 0;

    for (const auto& stream : m_VertexStreams)
      uiOffset += RHI::spPixelFormatHelper::GetSizeInBytes(stream.m_eFormat);

    auto& stream = m_VertexStreams.ExpandAndGetRef();
    stream.m_uiOffset = uiOffset;
    stream.m_eSemantic = eSemantic;
    stream.m_uiSemanticIndex = uiSemanticIndex;
    stream.m_eFormat = eFormat;

    m_VertexStreams.Sort();

    m_uiVertexSize = uiOffset + RHI::spPixelFormatHelper::GetSizeInBytes(eFormat);

    return m_VertexStreams.GetCount() - 1;
  }

  void spMeshDataBuilder::AllocateMeshData(spMesh::Data& ref_MeshData, ezUInt32 uiMaxVertexCount, const ezEnum<RHI::spPrimitiveTopology>& eTopology, ezUInt32 uiNumPrimitives, bool bZeroFill)
  {
    EZ_ASSERT_DEV(!m_VertexStreams.IsEmpty(), "Can't allocate mesh data without vertex streams.");

    ref_MeshData.m_Vertices.Clear();
    ref_MeshData.m_Indices.Clear();
    ref_MeshData.m_VertexStreams.Clear();

    ezUInt32 uiIndicesCount = RHI::spPrimitiveTopology::GetIndicesCount(eTopology, uiNumPrimitives);

    if (bZeroFill)
      ref_MeshData.m_Vertices.SetCount(m_uiVertexSize * uiMaxVertexCount);
    else
      ref_MeshData.m_Vertices.SetCountUninitialized(ref_MeshData.m_uiVertexSize * uiMaxVertexCount);

    ref_MeshData.m_Indices.SetCount(uiIndicesCount * sizeof(ezUInt16));

    ref_MeshData.m_eTopology = eTopology;
    ref_MeshData.m_uiVertexSize = m_uiVertexSize;

    ref_MeshData.m_VertexStreams.SetCountUninitialized(m_VertexStreams.GetCount());
    ezMemoryUtils::Copy(ref_MeshData.m_VertexStreams.GetData(), m_VertexStreams.GetData(), m_VertexStreams.GetCount());
  }

  ezResult spMeshDataUtils::EncodeFloat(const float& fSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    EZ_ASSERT_DEV(pDest.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid destination buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float1:
        *reinterpret_cast<float*>(pDest.GetPtr()) = fSource;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Half1:
        *reinterpret_cast<ezFloat16*>(pDest.GetPtr()) = fSource;
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }

  ezResult spMeshDataUtils::EncodeVec2(const ezVec2& vSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    EZ_ASSERT_DEV(pDest.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid destination buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float2:
        *reinterpret_cast<ezVec2*>(pDest.GetPtr()) = vSource;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Half2:
        *reinterpret_cast<ezFloat16Vec2*>(pDest.GetPtr()) = vSource;
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }

  ezResult spMeshDataUtils::EncodeVec3(const ezVec3& vSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    EZ_ASSERT_DEV(pDest.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid destination buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float3:
        *reinterpret_cast<ezVec3*>(pDest.GetPtr()) = vSource;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::UShort4Norm:
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[0] = ezMath::ColorFloatToShort(vSource.x);
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[1] = ezMath::ColorFloatToShort(vSource.y);
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[2] = ezMath::ColorFloatToShort(vSource.z);
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[3] = 0;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Short4Norm:
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[0] = ezMath::ColorFloatToSignedShort(vSource.x);
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[1] = ezMath::ColorFloatToSignedShort(vSource.y);
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[2] = ezMath::ColorFloatToSignedShort(vSource.z);
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[3] = 0;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Byte4Norm:
        pDest.GetPtr()[0] = ezMath::ColorFloatToByte(vSource.x);
        pDest.GetPtr()[1] = ezMath::ColorFloatToByte(vSource.y);
        pDest.GetPtr()[2] = ezMath::ColorFloatToByte(vSource.z);
        pDest.GetPtr()[3] = 0;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::R10G10B10A2UNorm:
        *reinterpret_cast<ezUInt32*>(pDest.GetPtr()) = ColorFloatToUNorm<10>(vSource.x);
        *reinterpret_cast<ezUInt32*>(pDest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.y) << 10;
        *reinterpret_cast<ezUInt32*>(pDest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.z) << 20;
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }

  ezResult spMeshDataUtils::EncodeVec4(const ezVec4& vSource, ezByteArrayPtr pDest, RHI::spInputElementFormat::Enum eFormat)
  {
    EZ_ASSERT_DEV(pDest.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid destination buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float4:
        *reinterpret_cast<ezVec4*>(pDest.GetPtr()) = vSource;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Half4:
        *reinterpret_cast<ezFloat16Vec4*>(pDest.GetPtr()) = vSource;
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::UShort4Norm:
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[0] = ezMath::ColorFloatToShort(vSource.x);
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[1] = ezMath::ColorFloatToShort(vSource.y);
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[2] = ezMath::ColorFloatToShort(vSource.z);
        reinterpret_cast<ezUInt16*>(pDest.GetPtr())[3] = ezMath::ColorFloatToShort(vSource.w);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Short4Norm:
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[0] = ezMath::ColorFloatToSignedShort(vSource.x);
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[1] = ezMath::ColorFloatToSignedShort(vSource.y);
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[2] = ezMath::ColorFloatToSignedShort(vSource.z);
        reinterpret_cast<ezInt16*>(pDest.GetPtr())[3] = ezMath::ColorFloatToSignedShort(vSource.w);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Byte4Norm:
        pDest.GetPtr()[0] = ezMath::ColorFloatToByte(vSource.x);
        pDest.GetPtr()[1] = ezMath::ColorFloatToByte(vSource.y);
        pDest.GetPtr()[2] = ezMath::ColorFloatToByte(vSource.z);
        pDest.GetPtr()[3] = ezMath::ColorFloatToByte(vSource.w);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::SByte4Norm:
        pDest.GetPtr()[0] = ezMath::ColorFloatToSignedByte(vSource.x);
        pDest.GetPtr()[1] = ezMath::ColorFloatToSignedByte(vSource.y);
        pDest.GetPtr()[2] = ezMath::ColorFloatToSignedByte(vSource.z);
        pDest.GetPtr()[3] = ezMath::ColorFloatToSignedByte(vSource.w);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::R10G10B10A2UNorm:
        *reinterpret_cast<ezUInt32*>(pDest.GetPtr()) = ColorFloatToUNorm<10>(vSource.x);
        *reinterpret_cast<ezUInt32*>(pDest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.y) << 10;
        *reinterpret_cast<ezUInt32*>(pDest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.z) << 20;
        *reinterpret_cast<ezUInt32*>(pDest.GetPtr()) |= ColorFloatToUNorm<2>(vSource.w) << 30;
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }

  ezResult spMeshDataUtils::DecodeFloat(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, float& ref_fDest)
  {
    EZ_ASSERT_DEV(pSource.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid source buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float1:
        ref_fDest = *reinterpret_cast<const float*>(pSource.GetPtr());
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Half1:
        ref_fDest = *reinterpret_cast<const ezFloat16*>(pSource.GetPtr());
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }

  ezResult spMeshDataUtils::DecodeVec2(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec2& ref_vDest)
  {
    EZ_ASSERT_DEV(pSource.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid source buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float2:
        ref_vDest = *reinterpret_cast<const ezVec2*>(pSource.GetPtr());
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Half2:
        ref_vDest = *reinterpret_cast<const ezFloat16Vec2*>(pSource.GetPtr());
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }

  ezResult spMeshDataUtils::DecodeVec3(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec3& ref_vDest)
  {
    EZ_ASSERT_DEV(pSource.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid source buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float3:
        ref_vDest = *reinterpret_cast<const ezVec3*>(pSource.GetPtr());
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::UShort4Norm:
        ref_vDest.x = ezMath::ColorShortToFloat(reinterpret_cast<const ezUInt16*>(pSource.GetPtr())[0]);
        ref_vDest.y = ezMath::ColorShortToFloat(reinterpret_cast<const ezUInt16*>(pSource.GetPtr())[1]);
        ref_vDest.z = ezMath::ColorShortToFloat(reinterpret_cast<const ezUInt16*>(pSource.GetPtr())[2]);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Short4Norm:
        ref_vDest.x = ezMath::ColorSignedShortToFloat(reinterpret_cast<const ezInt16*>(pSource.GetPtr())[0]);
        ref_vDest.y = ezMath::ColorSignedShortToFloat(reinterpret_cast<const ezInt16*>(pSource.GetPtr())[1]);
        ref_vDest.z = ezMath::ColorSignedShortToFloat(reinterpret_cast<const ezInt16*>(pSource.GetPtr())[2]);
        return EZ_SUCCESS;


      case RHI::spInputElementFormat::Byte4Norm:
        ref_vDest.x = ezMath::ColorByteToFloat(pSource.GetPtr()[0]);
        ref_vDest.y = ezMath::ColorByteToFloat(pSource.GetPtr()[1]);
        ref_vDest.z = ezMath::ColorByteToFloat(pSource.GetPtr()[2]);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::SByte4Norm:
        ref_vDest.x = ezMath::ColorSignedByteToFloat(pSource.GetPtr()[0]);
        ref_vDest.y = ezMath::ColorSignedByteToFloat(pSource.GetPtr()[1]);
        ref_vDest.z = ezMath::ColorSignedByteToFloat(pSource.GetPtr()[2]);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::R10G10B10A2UNorm:
        ref_vDest.x = ColorUNormToFloat<10>(*reinterpret_cast<const ezUInt32*>(pSource.GetPtr()));
        ref_vDest.y = ColorUNormToFloat<10>(*reinterpret_cast<const ezUInt32*>(pSource.GetPtr()) >> 10);
        ref_vDest.z = ColorUNormToFloat<10>(*reinterpret_cast<const ezUInt32*>(pSource.GetPtr()) >> 20);
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }

  ezResult spMeshDataUtils::DecodeVec4(const ezConstByteArrayPtr& pSource, RHI::spInputElementFormat::Enum eFormat, ezVec4& ref_vDest)
  {
    EZ_ASSERT_DEV(pSource.GetCount() >= RHI::spPixelFormatHelper::GetSizeInBytes(eFormat), "Invalid source buffer size.");

    switch (eFormat)
    {
      case RHI::spInputElementFormat::Float4:
        ref_vDest = *reinterpret_cast<const ezVec4*>(pSource.GetPtr());
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Half4:
        ref_vDest = *reinterpret_cast<const ezFloat16Vec4*>(pSource.GetPtr());
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::UShort4Norm:
        ref_vDest.x = ezMath::ColorShortToFloat(reinterpret_cast<const ezUInt16*>(pSource.GetPtr())[0]);
        ref_vDest.y = ezMath::ColorShortToFloat(reinterpret_cast<const ezUInt16*>(pSource.GetPtr())[1]);
        ref_vDest.z = ezMath::ColorShortToFloat(reinterpret_cast<const ezUInt16*>(pSource.GetPtr())[2]);
        ref_vDest.w = ezMath::ColorShortToFloat(reinterpret_cast<const ezUInt16*>(pSource.GetPtr())[3]);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Short4Norm:
        ref_vDest.x = ezMath::ColorSignedShortToFloat(reinterpret_cast<const ezInt16*>(pSource.GetPtr())[0]);
        ref_vDest.y = ezMath::ColorSignedShortToFloat(reinterpret_cast<const ezInt16*>(pSource.GetPtr())[1]);
        ref_vDest.z = ezMath::ColorSignedShortToFloat(reinterpret_cast<const ezInt16*>(pSource.GetPtr())[2]);
        ref_vDest.w = ezMath::ColorSignedShortToFloat(reinterpret_cast<const ezInt16*>(pSource.GetPtr())[3]);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::Byte4Norm:
        ref_vDest.x = ezMath::ColorByteToFloat(pSource.GetPtr()[0]);
        ref_vDest.y = ezMath::ColorByteToFloat(pSource.GetPtr()[1]);
        ref_vDest.z = ezMath::ColorByteToFloat(pSource.GetPtr()[2]);
        ref_vDest.w = ezMath::ColorByteToFloat(pSource.GetPtr()[3]);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::SByte4Norm:
        ref_vDest.x = ezMath::ColorSignedByteToFloat(pSource.GetPtr()[0]);
        ref_vDest.y = ezMath::ColorSignedByteToFloat(pSource.GetPtr()[1]);
        ref_vDest.z = ezMath::ColorSignedByteToFloat(pSource.GetPtr()[2]);
        ref_vDest.w = ezMath::ColorSignedByteToFloat(pSource.GetPtr()[3]);
        return EZ_SUCCESS;

      case RHI::spInputElementFormat::R10G10B10A2UNorm:
        ref_vDest.x = ColorUNormToFloat<10>(*reinterpret_cast<const ezUInt32*>(pSource.GetPtr()));
        ref_vDest.y = ColorUNormToFloat<10>(*reinterpret_cast<const ezUInt32*>(pSource.GetPtr()) >> 10);
        ref_vDest.z = ColorUNormToFloat<10>(*reinterpret_cast<const ezUInt32*>(pSource.GetPtr()) >> 20);
        ref_vDest.w = ColorUNormToFloat<2>(*reinterpret_cast<const ezUInt32*>(pSource.GetPtr()) >> 30);
        return EZ_SUCCESS;

      default:
        return EZ_FAILURE;
    }
  }
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Mesh);
