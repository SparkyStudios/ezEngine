// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spShaderSpecializationConstant& constant)
{
  inout_stream << constant.m_eType;
  inout_stream << constant.m_sName;
  inout_stream << constant.m_uiValue;

  return inout_stream;
}

inline void operator>>(ezStreamReader& inout_stream, RPI::spShaderSpecializationConstant& out_constant)
{
  inout_stream >> out_constant.m_eType;
  inout_stream >> out_constant.m_sName;
  inout_stream >> out_constant.m_uiValue;
}
