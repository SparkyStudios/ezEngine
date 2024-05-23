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

#include <RPI/Camera/CameraSlot.h>
#include <RPI/Core/Renderer.h>

namespace RPI
{
  class SP_RPI_DLL spClearRenderer : public spRenderer
  {
    EZ_ADD_DYNAMIC_REFLECTION(spClearRenderer, spRenderer);

    // spRenderer

  public:
    void Render() override;
    void Prepare() override;

    // spClearRenderer

  public:
    struct ClearFlags
    {
      typedef ezUInt8 StorageType;

      enum Enum : StorageType
      {
        Color = EZ_BIT(0),
        Depth = EZ_BIT(1),

        Default = Color | Depth,
      };

      struct Bits
      {
        StorageType Color : 1;
        StorageType Depth : 1;
      };
    };

    spClearRenderer();
    ~spClearRenderer() override = default;

#pragma region Properties

    /// \brief Gets the color to clear the render target to.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezColor GetClearColor() const { return m_ClearColor; }

    /// \brief Sets the color to clear the render target to.
    /// \param color The color to clear the render target to.
    EZ_ALWAYS_INLINE void SetClearColor(const ezColor& color) { m_ClearColor = color; }

    /// \brief Gets the depth value used to clear the depth texture.
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetClearDepth() const { return m_fClearDepth; }

    /// \brief Sets the depth value used to clear the depth texture.
    /// \param fDepth The depth value to clear the depth texture to.
    EZ_ALWAYS_INLINE void SetClearDepth(float fDepth) { m_fClearDepth = fDepth; }

    /// \brief Gets the stencil value used to clear the stencil texture.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt8 GetClearStencil() const { return m_uiClearStencil; }

    /// \brief Sets the stencil value used to clear the stencil texture.
    /// \param uiStencil The stencil value to clear the stencil texture to.
    EZ_ALWAYS_INLINE void SetClearStencil(ezUInt8 uiStencil) { m_uiClearStencil = uiStencil; }

    /// \brief Gets the clear flags.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezBitflags<ClearFlags> GetClearFlags() const { return m_eClearFlags; }

    /// \brief Sets the clear flags.
    /// \param eFlags The clear flags to set.
    EZ_ALWAYS_INLINE void SetClearFlags(ezBitflags<ClearFlags> eFlags) { m_eClearFlags = eFlags; }

#pragma endregion

  private:
    ezBitflags<ClearFlags> m_eClearFlags{ClearFlags::Default};
    ezColor m_ClearColor{ezColor::SkyBlue};
    float m_fClearDepth{1.0f};
    ezUInt8 m_uiClearStencil{0};
  };
} // namespace RPI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spClearRenderer::ClearFlags);
