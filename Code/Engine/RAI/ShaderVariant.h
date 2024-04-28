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

#include <RAI/RAIDLL.h>

#include <Foundation/IO/MemoryStream.h>

#include <RHI/Input.h>
#include <RHI/Rendering.h>
#include <RHI/Shader.h>

namespace RAI
{
  /// \brief A shader variant asset.
  ///
  /// Reflects an SPSL binary shader variant.
  class SP_RAI_DLL spShaderVariant
  {
    friend class spShader;
    friend class spShaderVariantResource;
    friend class spShaderVariantResourceDescriptor;

#pragma region RAI Resource

  public:
    spShaderVariant() = default;

    explicit spShaderVariant(ezStringView sName);

    ~spShaderVariant() noexcept;

    void Clear();

    ezUInt32 m_uiHash;

    ezHashedString m_sName;

    ezHybridArray<RHI::spInputElementDescription, 8> m_InputElements;

    ezDynamicArray<ezUInt8> m_Buffer;

    ezMap<ezEnum<RHI::spShaderStage>, ezHashedString> m_EntryPoints;

    ezHybridArray<spPermutationVar, 16> m_Permutations;

    RHI::spRenderingState m_RenderingState;

    ezEnum<RHI::spShaderLanguage> m_eShaderLanguage;

#pragma endregion

#pragma region RHI Resources

  public:
    /// \brief Creates the RHI shader program resource from all the shader stages
    /// owned by this shader.
    void CreateRHIShaderProgram();

    /// \brief Returns the RHI shader program.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spShaderProgram> GetRHIShaderProgram() const { return m_pRHIShaderProgram; }

    /// \brief Gets the shader description for a specific shader stage.
    /// \param [in] eStage The shader stage to get the description for.
    /// \param [out] out_description The shader description for the given stage.
    EZ_NODISCARD ezResult GetRHIShaderDescription(const ezEnum<RHI::spShaderStage>& eStage, RHI::spShaderDescription& out_description) const;

  private:
    ezSharedPtr<RHI::spShaderProgram> m_pRHIShaderProgram{nullptr};

#pragma endregion
  };
} // namespace RAI

#include <RAI/Implementation/ShaderVariant_inl.h>
