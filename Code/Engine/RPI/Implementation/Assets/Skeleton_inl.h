// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spSkeleton& skeleton)
{
  inout_stream.WriteArray(skeleton.GetJoints()).IgnoreResult();
  return inout_stream;
}

EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spSkeleton& skeleton)
{
  inout_stream.ReadArray(skeleton.GetJoints()).IgnoreResult();
  return inout_stream;
}

EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spSkeleton::Joint& joint)
{
  inout_stream << joint.m_uiIndex;
  inout_stream << joint.m_uiParentIndex;
  inout_stream << joint.m_sName;
  inout_stream << joint.m_Transform;

  return inout_stream;
}

EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spSkeleton::Joint& ref_joint)
{
  inout_stream >> ref_joint.m_uiIndex;
  inout_stream >> ref_joint.m_uiParentIndex;
  inout_stream >> ref_joint.m_sName;
  inout_stream >> ref_joint.m_Transform;

  return inout_stream;
}
