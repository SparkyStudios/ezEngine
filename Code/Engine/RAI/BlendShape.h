#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Core.h>

namespace RAI
{
  /// \brief A blend shape asset.
  ///
  /// A blend shape is used when doing morph target animations. This asset
  /// stores the data for a single blend shape within a mesh. That blend shape
  /// may be compatible with other meshes other than the source mesh.
  class SP_RAI_DLL spBlendShape
  {
    friend class spBlendShapeResource;
    friend class spBlendShapeResourceDescriptor;

  public:
    /// \brief The blend shape name.
    ezHashedString m_sName;

    /// \brief The blend shape vertices.
    ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper> m_Vertices;
    ezUInt32 m_uiVertexSize;

    /// \brief The blend shape weight. This affects how much the vertices are blended together.
    float m_fWeight{0};
  };
} // namespace RAI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spBlendShape& blendShape)
{
  inout_stream << blendShape.m_sName;
  inout_stream.WriteArray(blendShape.m_Vertices).AssertSuccess();
  inout_stream << blendShape.m_uiVertexSize;
  inout_stream << blendShape.m_fWeight;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spBlendShape& ref_blendShape)
{
  inout_stream >> ref_blendShape.m_sName;
  inout_stream.ReadArray(ref_blendShape.m_Vertices).AssertSuccess();
  inout_stream >> ref_blendShape.m_uiVertexSize;
  inout_stream >> ref_blendShape.m_fWeight;

  return inout_stream;
}
