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

namespace RPI
{
  typedef ezGenericId<24, 8> spCameraSlotHandleId;

  /// \brief A handle to a \a spCameraSlot.
  class spCameraSlotHandle
  {
    EZ_DECLARE_HANDLE_TYPE(spCameraSlotHandle, spCameraSlotHandleId);
  };

  /// \brief A camera slot in a \a spCompositor.
  ///
  /// This is used to assign a pipeline to a camera. A camera slot can only be assigned to a single pipeline
  /// at a time, but many cameras can be assigned to the same slot at the same time.
  ///
  /// A \a spCamera can dynamically change its slot at runtime, and the new slot will be active only at the
  /// next frame.
  ///
  /// \see spCompositor
  /// \see spCamera
  class SP_RPI_DLL spCameraSlot : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spCameraSlot, ezReflectedClass);

  public:
    spCameraSlot() = default;
    ~spCameraSlot() override = default;

    /// \brief Gets the handle of this slot.
    EZ_NODISCARD EZ_ALWAYS_INLINE spCameraSlotHandle GetHandle() const { return m_Handle; }

    /// \brief Gets the name of the slot.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezStringView GetName() const { return m_sName; }

    /// \brief Sets the name of the slot.
    /// \param sName The new name of the slot.
    EZ_ALWAYS_INLINE virtual void SetName(ezStringView sName)
    {
      if (!sName.IsEmpty())
        m_sName.Assign(sName);
    }

  private:
    spCameraSlotHandle m_Handle;

    ezHashedString m_sName;
  };
} // namespace RPI
