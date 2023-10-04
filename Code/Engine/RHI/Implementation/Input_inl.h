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

namespace RHI
{
  inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RHI::spInputElementDescription& description)
  {
    inout_stream << description.m_sName;
    inout_stream << description.m_eSemantic;
    inout_stream << description.m_eFormat;
    inout_stream << description.m_uiOffset;

    return inout_stream;
  }

  inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RHI::spInputElementDescription& ref_description)
  {
    inout_stream >> ref_description.m_sName;
    inout_stream >> ref_description.m_eSemantic;
    inout_stream >> ref_description.m_eFormat;
    inout_stream >> ref_description.m_uiOffset;

    return inout_stream;
  }

  inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RHI::spInputLayoutDescription& description)
  {
    inout_stream << description.m_uiStride;
    inout_stream.WriteArray(description.m_Elements).IgnoreResult();
    inout_stream << description.m_uiInstanceStepRate;

    return inout_stream;
  }

  inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RHI::spInputLayoutDescription& ref_description)
  {
    inout_stream >> ref_description.m_uiStride;
    inout_stream.ReadArray(ref_description.m_Elements).IgnoreResult();
    inout_stream >> ref_description.m_uiInstanceStepRate;

    return inout_stream;
  }
} // namespace RHI
