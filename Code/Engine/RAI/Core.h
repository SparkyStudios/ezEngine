#pragma once

/// \brief Represents an invalid joint index.
#define spSkeletonInvalidJointIndex static_cast<ezUInt16>(0xFFFFu)

enum SP_RAI_CONSTANTS : ezUInt8
{
  /// \brief The maximum number of lod a mesh can have.
  SP_RAI_MAX_LOD_COUNT = 5
};

namespace RAI
{
  /// \brief A single vertex in a 3D mesh.
  struct spVertex
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

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const spVertex& other) const
    {
      return m_vPosition == other.m_vPosition && m_vNormal == other.m_vNormal
      && m_vTangent == other.m_vTangent && m_vBiTangent == other.m_vBiTangent
      && m_vTexCoord0 == other.m_vTexCoord0 && m_vTexCoord1 == other.m_vTexCoord1
      && m_Color0 == other.m_Color0 && m_Color1 == other.m_Color1;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const spVertex& other) const
    {
      return !(*this == other);
    }
  };


  inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spVertex& vertex)
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

  inline ezStreamReader& operator>>(ezStreamReader& inout_stream, spVertex& ref_vertex)
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

} // namespace RAI
