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

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spBlendShape& blendShape)
{
  inout_stream << blendShape.m_sName;
  inout_stream.WriteArray(blendShape.m_Vertices).AssertSuccess();
  inout_stream << blendShape.m_uiVertexSize;
  inout_stream << blendShape.m_fWeight;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spBlendShape& ref_blendShape)
{
  inout_stream >> ref_blendShape.m_sName;
  inout_stream.ReadArray(ref_blendShape.m_Vertices).AssertSuccess();
  inout_stream >> ref_blendShape.m_uiVertexSize;
  inout_stream >> ref_blendShape.m_fWeight;

  return inout_stream;
}
