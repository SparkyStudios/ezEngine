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

#include <RAI/Resources/ImageResource.h>
#include <RAI/Resources/SamplerResource.h>

#include <RHI/Texture.h>

#include <Foundation/IO/MemoryStream.h>

namespace RAI
{
  /// \brief A texture asset.
  ///
  /// A texture asset is a combination of an image resource and a sampler
  /// resource. Once loaded, it will create a texture object in the RHI layer
  /// and can then be used for rendering.
  class SP_RAI_DLL spTexture
  {
    friend class spTexture2DResource;

    friend class spTextureResourceLoader;
    friend class spTextureResourceDescriptor;

  public:
    spTexture() = default;

    explicit spTexture(RHI::spTextureDescription description)
      : m_TextureDescription(std::move(description))
    {
    }

    /// \brief Gets the RHI layer's texture description.
    EZ_NODISCARD EZ_ALWAYS_INLINE const RHI::spTextureDescription& GetDescription() const { return m_TextureDescription; }

    /// \brief Sets the RHI layer's texture description.
    EZ_ALWAYS_INLINE void SetDescription(RHI::spTextureDescription description) { m_TextureDescription = std::move(description); }

    /// \brief Gets the image asset associated with this texture.
    EZ_NODISCARD EZ_ALWAYS_INLINE const spImageResourceHandle& GetImage() const { return m_hImage; }

    /// \brief Sets the image asset associated with this texture.
    EZ_ALWAYS_INLINE void SetImage(const spImageResourceHandle& hImage) { m_hImage = hImage; }

    /// \brief Gets the sampler asset associated with this texture.
    EZ_NODISCARD EZ_ALWAYS_INLINE const spSamplerResourceHandle& GetSampler() const { return m_hSampler; }

    /// \brief Sets the sampler asset associated with this texture.
    EZ_ALWAYS_INLINE void SetSampler(const spSamplerResourceHandle& hSampler) { m_hSampler = hSampler; }

  private:
    RHI::spTextureDescription m_TextureDescription;

    spImageResourceHandle m_hImage;
    spSamplerResourceHandle m_hSampler;
  };
} // namespace RAI
