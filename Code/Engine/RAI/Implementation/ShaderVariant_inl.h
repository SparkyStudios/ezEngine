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

namespace RAI
{
  EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spShaderVariant variant)
  {
    inout_stream << variant.m_sName;
    inout_stream.WriteArray(variant.m_InputElements).AssertSuccess();
    inout_stream.WriteArray(variant.m_Buffer).AssertSuccess();
    inout_stream.WriteMap(variant.m_EntryPoints).AssertSuccess();
    inout_stream.WriteArray(variant.m_Permutations).AssertSuccess();
    inout_stream.WriteBytes(&variant.m_RenderingState, sizeof(RHI::spRenderingState)).AssertSuccess();
    inout_stream.WriteBytes(&variant.m_eShaderLanguage, sizeof(RHI::spShaderLanguage)).AssertSuccess();

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, spShaderVariant& variant)
  {
    inout_stream >> variant.m_sName;
    inout_stream.ReadArray(variant.m_InputElements).AssertSuccess();
    inout_stream.ReadArray(variant.m_Buffer).AssertSuccess();
    inout_stream.ReadMap(variant.m_EntryPoints).AssertSuccess();
    inout_stream.ReadArray(variant.m_Permutations).AssertSuccess();
    inout_stream.ReadBytes(&variant.m_RenderingState, sizeof(RHI::spRenderingState));
    inout_stream.ReadBytes(&variant.m_eShaderLanguage, sizeof(RHI::spShaderLanguage));

    return inout_stream;
  }
} // namespace RAI
