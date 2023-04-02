#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Core.h>

namespace RAI
{
  class SP_RAI_DLL spSkeleton
  {
    friend class spSkeletonResourceDescriptor;
    friend class spSkeletonResource;

  public:
    /// \brief A single joint in the skeleton hierarchy.
    struct Joint
    {
      ezUInt16 m_uiIndex{spSkeletonInvalidJointIndex};
      ezUInt16 m_uiParentIndex{spSkeletonInvalidJointIndex};
      ezHashedString m_sName;
      ezTransform m_Transform;
    };

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<Joint>& GetJoints() const { return m_Joints; }

    EZ_ALWAYS_INLINE ezDynamicArray<Joint>& GetJoints() { return m_Joints; }

  private:
    ezDynamicArray<Joint> m_Joints;
  };
} // namespace RAI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spSkeleton::Joint& joint)
{
  inout_stream << joint.m_uiIndex;
  inout_stream << joint.m_uiParentIndex;
  inout_stream << joint.m_sName;
  inout_stream << joint.m_Transform;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spSkeleton::Joint& ref_joint)
{
  inout_stream >> ref_joint.m_uiIndex;
  inout_stream >> ref_joint.m_uiParentIndex;
  inout_stream >> ref_joint.m_sName;
  inout_stream >> ref_joint.m_Transform;

  return inout_stream;
}
