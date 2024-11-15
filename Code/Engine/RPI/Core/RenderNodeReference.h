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

namespace RPI
{
  /// \brief A struct used to create and hold references to render nodes.
  ///
  /// These references are most of the time indices to a render node from
  /// an array.
  struct SP_RPI_DLL spRenderNodeReference
  {
  public:
    EZ_DECLARE_POD_TYPE();

    /// \brief Makes an invalid reference.
    static spRenderNodeReference MakeInvalid();

    /// \brief Makes a reference to a node.
    /// \param [in] iReference The reference to the node.
    explicit spRenderNodeReference(ezUInt32 iReference);

    /// \brief Gets the reference. It's typically the index of the render node
    /// in the collection it is associated with.
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetRef() const { return m_iRef; }

    /// \brief Checks if the reference is invalid.
    [[nodiscard]] EZ_ALWAYS_INLINE bool IsInvalid() const { return m_iRef == ezInvalidIndex; }

    [[nodiscard]] EZ_ALWAYS_INLINE bool operator==(const spRenderNodeReference& rhs) const
    {
      return m_iRef == rhs.m_iRef;
    }

    [[nodiscard]] EZ_ALWAYS_INLINE bool operator!=(const spRenderNodeReference& rhs) const
    {
      return m_iRef != rhs.m_iRef;
    }

    [[nodiscard]] EZ_ALWAYS_INLINE bool operator<(const spRenderNodeReference& rhs) const
    {
      return m_iRef < rhs.m_iRef;
    }

    [[nodiscard]] EZ_ALWAYS_INLINE spRenderNodeReference operator+(const ezUInt32 rhs) const
    {
      return spRenderNodeReference(m_iRef + rhs);
    }

    [[nodiscard]] EZ_ALWAYS_INLINE spRenderNodeReference operator*(const ezUInt32 rhs) const
    {
      return spRenderNodeReference(m_iRef * rhs);
    }

  private:
    ezUInt32 m_iRef{ezInvalidIndex};
  };
}
