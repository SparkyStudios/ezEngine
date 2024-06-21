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

#pragma once

#include <RPI/RPIDLL.h>

#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Transform.h>

/// \brief An alias for the \c ezVec2 class used for shaders.
typedef ezVec2 spShaderVec2;

/// \brief An alias for the \c ezVec3 class used for shaders.
typedef ezVec3 spShaderVec3;

/// \brief A wrapper class that converts a ezMat3 into the correct data layout for shaders.
class spShaderMat3
{
public:
  EZ_DECLARE_POD_TYPE();

  EZ_ALWAYS_INLINE spShaderMat3() = default;

  EZ_ALWAYS_INLINE spShaderMat3(const ezMat3& m) { *this = m; }

  EZ_FORCE_INLINE spShaderMat3& operator=(const ezMat3& m)
  {
    for (ezUInt32 c = 0; c < 3; ++c)
    {
      m_Data[c * 4 + 0] = m.Element(c, 0);
      m_Data[c * 4 + 1] = m.Element(c, 1);
      m_Data[c * 4 + 2] = m.Element(c, 2);
      m_Data[c * 4 + 3] = 0.0f;
    }

    return *this;
  }

private:
  float m_Data[12];
};

/// \brief An alias for the \c ezMat4 class used for shaders.
typedef ezMat4 spShaderMat4;

/// \brief A wrapper class that converts a ezTransform into the correct data layout for shaders.
class spShaderTransform
{
public:
  EZ_DECLARE_POD_TYPE();

  EZ_ALWAYS_INLINE spShaderTransform() = default;

  EZ_ALWAYS_INLINE spShaderTransform(const ezTransform& t) { *this = t; }

  EZ_FORCE_INLINE void operator=(const ezTransform& t) { *this = t.GetAsMat4(); }

  EZ_FORCE_INLINE spShaderTransform& operator=(const ezMat4& t)
  {
    float data[16];
    t.GetAsArray(data, ezMatrixLayout::RowMajor);

    for (ezUInt32 i = 0; i < 12; ++i)
      m_Data[i] = data[i];

    return *this;
  }

  EZ_FORCE_INLINE spShaderTransform& operator=(const ezMat3& t)
  {
    float data[9];
    t.GetAsArray(data, ezMatrixLayout::RowMajor);

    m_Data[0] = data[0];
    m_Data[1] = data[1];
    m_Data[2] = data[2];
    m_Data[3] = 0;

    m_Data[4] = data[3];
    m_Data[5] = data[4];
    m_Data[6] = data[5];
    m_Data[7] = 0;

    m_Data[8] = data[6];
    m_Data[9] = data[7];
    m_Data[10] = data[8];
    m_Data[11] = 0;

    return *this;
  }

  [[nodiscard]] EZ_FORCE_INLINE ezMat4 GetAsMat4() const
  {
    ezMat4 res;
    res.SetRow(0, reinterpret_cast<const ezVec4&>(m_Data[0]));
    res.SetRow(1, reinterpret_cast<const ezVec4&>(m_Data[4]));
    res.SetRow(2, reinterpret_cast<const ezVec4&>(m_Data[8]));
    res.SetRow(3, ezVec4(0, 0, 0, 1));

    return res;
  }

  [[nodiscard]] EZ_FORCE_INLINE ezVec3 GetTranslationVector() const
  {
    return ezVec3(m_Data[3], m_Data[7], m_Data[11]);
  }

private:
  float m_Data[12];
};

/// \brief A wrapper class that converts a bool into the correct data layout for shaders.
class spShaderBool
{
public:
  EZ_DECLARE_POD_TYPE();

  EZ_ALWAYS_INLINE spShaderBool() = default;

  EZ_ALWAYS_INLINE spShaderBool(bool b) { *this = b; }

  EZ_ALWAYS_INLINE spShaderBool operator=(bool b)
  {
    m_uiData = b ? 0xFFFFFFFF : 0;
    return *this;
  }

private:
  ezUInt32 m_uiData;
};
