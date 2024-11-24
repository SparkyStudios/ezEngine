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

#include <RPI/RPIDLL.h>

#include <RPI/Core.h>

namespace RPI
{
  /// \brief A skeleton asset.
  ///
  /// A skeleton is a collection of joints that can be animated. Each joint
  /// stores the name of the joint, the parent joint, and the transformation
  /// matrix of its bind pose.
  class SP_RPI_DLL spSkeleton
  {
    friend class spSkeletonResourceDescriptor;
    friend class spSkeletonResource;

  public:
    /// \brief A single joint in the skeleton hierarchy.
    struct Joint
    {
      /// \brief The index of this joint in the skeleton.
      ezUInt16 m_uiIndex{spSkeletonInvalidJointIndex};

      /// \brief The index of the parent of this joint in the skeleton.
      /// Set it to \a spSkeletonInvalidJointIndex if this joint is the root joint.
      ezUInt16 m_uiParentIndex{spSkeletonInvalidJointIndex};

      /// \brief The name of this joint.
      ezHashedString m_sName;

      /// \brief The bind pose transformation matrix of this joint.
      ezTransform m_Transform;
    };

    /// \brief Gets the joints of the skeleton.
    [[nodiscard]] EZ_ALWAYS_INLINE const ezDynamicArray<Joint>& GetJoints() const { return m_Joints; }

    /// \brief Gets the joints of the skeleton.
    EZ_ALWAYS_INLINE ezDynamicArray<Joint>& GetJoints() { return m_Joints; }

  private:
    ezDynamicArray<Joint> m_Joints;
  };
} // namespace RPI

#include <RPI/Implementation/Assets/Skeleton_inl.h>
