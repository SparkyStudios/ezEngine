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

#include <RHI/Texture.h>

#include <Foundation/IO/MemoryStream.h>

namespace RAI
{
  /// \brief An image asset.
  ///
  /// Images are raw pixel data that are stored in memory. They can be
  /// compressed or not. Image assets should be used with samplers assets
  /// to create textures.
  ///
  /// Multiple textures can be created from the same image asset even with
  /// different samplers, if so, they will share the same pixel data.
  class SP_RAI_DLL spImage
  {
    friend class spImageResource;
    friend class spImageResourceDescriptor;

    friend class spTextureResourceLoader;

  public:
    spImage() = default;

    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetWidth(ezUInt32 uiMipLevel = 0) const { return RHI::spTextureHelper::GetMipDimension(m_uiWidth, uiMipLevel); }
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetHeight(ezUInt32 uiMipLevel = 0) const { return RHI::spTextureHelper::GetMipDimension(m_uiHeight, uiMipLevel); }
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetDepth(ezUInt32 uiMipLevel = 0) const { return RHI::spTextureHelper::GetMipDimension(m_uiDepth, uiMipLevel); }
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetMipCount() const { return m_uiMipCount; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetArrayLayerCount() const { return m_uiArrayLayers; }
    [[nodiscard]] EZ_ALWAYS_INLINE const ezEnum<RHI::spPixelFormat>& GetPixelFormat() const { return m_eFormat; }

    EZ_ALWAYS_INLINE void SetWidth(ezUInt32 value) { m_uiWidth = value; }
    EZ_ALWAYS_INLINE void SetHeight(ezUInt32 value) { m_uiHeight = value; }
    EZ_ALWAYS_INLINE void SetDepth(ezUInt32 value) { m_uiDepth = value; }
    EZ_ALWAYS_INLINE void SetMipCount(ezUInt32 value) { m_uiMipCount = value; }
    EZ_ALWAYS_INLINE void SetArrayLayerCount(ezUInt32 value) { m_uiArrayLayers = value; }
    EZ_ALWAYS_INLINE void SetPixelFormat(const ezEnum<RHI::spPixelFormat>& value) { m_eFormat = value; }

    [[nodiscard]] EZ_ALWAYS_INLINE const ezContiguousMemoryStreamStorage& GetData() const { return m_Storage; }
    EZ_ALWAYS_INLINE ezContiguousMemoryStreamStorage& GetData() { return m_Storage; }

    [[nodiscard]] ezByteBlobPtr GetImageData(ezUInt32 uiMipLevel = 0, ezUInt32 uiDepth = 0, ezUInt32 uiArrayLayer = 0) const;

  private:
    ezResult LoadImageData();
    void UnloadImageData();

    /// \brief The width of the image.
    ezUInt32 m_uiWidth{0};

    /// \brief The height of the image.
    ezUInt32 m_uiHeight{0};

    /// \brief The depth of the image.
    ezUInt32 m_uiDepth{0};

    /// \brief The number of mipmaps in the image.
    ezUInt32 m_uiMipCount{0};

    /// \brief The number of array layers in the image.
    ezUInt32 m_uiArrayLayers{0};

    /// \brief The format of each individual image element.
    ezEnum<RHI::spPixelFormat> m_eFormat;

    void* m_pKtxTexture{nullptr};
    ezContiguousMemoryStreamStorage m_Storage;
  };
} // namespace RAI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spImage& image)
{
  inout_stream << image.GetWidth();
  inout_stream << image.GetHeight();
  inout_stream << image.GetDepth();
  inout_stream << image.GetMipCount();
  inout_stream << image.GetArrayLayerCount();
  inout_stream << image.GetPixelFormat();

  image.GetData().CopyToStream(inout_stream).AssertSuccess("Failed to write image data to stream.");

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spImage& ref_image)
{
  ezUInt32 uiWidth, uiHeight, uiDepth, uiMipCount, uiArrayLayers;
  ezEnum<RHI::spPixelFormat> ePixelFormat;

  inout_stream >> uiWidth;
  inout_stream >> uiHeight;
  inout_stream >> uiDepth;
  inout_stream >> uiMipCount;
  inout_stream >> uiArrayLayers;
  inout_stream >> ePixelFormat;

  ref_image.SetWidth(uiWidth);
  ref_image.SetHeight(uiHeight);
  ref_image.SetDepth(uiDepth);
  ref_image.SetMipCount(uiMipCount);
  ref_image.SetArrayLayerCount(uiArrayLayers);
  ref_image.SetPixelFormat(ePixelFormat);

  ref_image.GetData().ReadAll(inout_stream);

  return inout_stream;
}
