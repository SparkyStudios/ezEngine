#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Resource.h>

class spTexture;

/// \brief Describes a \see spTexture resource, for creation with a \see spDeviceResourceFactory.
struct spTextureDescription : public ezHashableStruct<spTextureDescription>
{
  /// \brief Creates a new \see spTextureDescription for a 1D \see spTexture resource.
  /// \param uiWidth The width of the texture in pixels.
  /// \param uiArrayLayers The number of layers of the texture.
  /// \param eFormat The format of the texture data.
  /// \param eUsage The usage of the texture.
  /// \param eSampleCount The number of samples in the texture.
  EZ_ALWAYS_INLINE static spTextureDescription Texture1D(
    ezUInt32 uiWidth,
    ezUInt32 uiMipCount,
    ezUInt32 uiArrayLayers,
    ezEnum<spPixelFormat> eFormat,
    ezBitflags<spTextureUsage> eUsage,
    ezEnum<spTextureSampleCount> eSampleCount = spTextureSampleCount::None)
  {
    return {
      {},
      uiWidth,
      1,
      1,
      uiMipCount,
      uiArrayLayers,
      eFormat,
      eUsage,
      spTextureDimension::Texture1D,
      eSampleCount,
    };
  }

  /// \brief Creates a new \see spTextureDescription for a 2D \see spTexture resource.
  /// \param uiWidth The width of the texture in pixels.
  /// \param uiHeight The height of the texture in pixels.
  /// \param uiArrayLayers The number of layers of the texture.
  /// \param eFormat The format of the texture data.
  /// \param eUsage The usage of the texture.
  /// \param eSampleCount The number of samples in the texture.
  EZ_ALWAYS_INLINE static spTextureDescription Texture2D(
    ezUInt32 uiWidth,
    ezUInt32 uiHeight,
    ezUInt32 uiMipCount,
    ezUInt32 uiArrayLayers,
    ezEnum<spPixelFormat> eFormat,
    ezBitflags<spTextureUsage> eUsage,
    ezEnum<spTextureSampleCount> eSampleCount = spTextureSampleCount::None)
  {
    return {
      {},
      uiWidth,
      uiHeight,
      1,
      uiMipCount,
      uiArrayLayers,
      eFormat,
      eUsage,
      spTextureDimension::Texture2D,
      eSampleCount,
    };
  }

  /// \brief Creates a new \see spTextureDescription for a 3D \see spTexture resource.
  /// \param uiWidth The width of the texture in pixels.
  /// \param uiHeight The height of the texture in pixels.
  /// \param uiDepth The depth of the texture in pixels.
  /// \param uiArrayLayers The number of layers of the texture.
  /// \param eFormat The format of the texture data.
  /// \param eUsage The usage of the texture.
  /// \param eSampleCount The number of samples in the texture.
  EZ_ALWAYS_INLINE static spTextureDescription Texture3D(
    ezUInt32 uiWidth,
    ezUInt32 uiHeight,
    ezUInt32 uiDepth,
    ezUInt32 uiMipCount,
    ezUInt32 uiArrayLayers,
    ezEnum<spPixelFormat> eFormat,
    ezBitflags<spTextureUsage> eUsage,
    ezEnum<spTextureSampleCount> eSampleCount = spTextureSampleCount::None)
  {
    return {
      {},
      uiWidth,
      uiHeight,
      uiDepth,
      uiMipCount,
      uiArrayLayers,
      eFormat,
      eUsage,
      spTextureDimension::Texture3D,
      eSampleCount,
    };
  }

  /// \brief Compares this instance with the specified \a other instance for equality.
  EZ_ALWAYS_INLINE bool operator==(const spTextureDescription& other) const
  {
    return m_uiWidth == other.m_uiWidth && m_uiHeight == other.m_uiHeight && m_uiDepth == other.m_uiDepth && m_uiMipCount == other.m_uiMipCount && m_uiArrayLayers == other.m_uiArrayLayers && m_eFormat == other.m_eFormat && m_eUsage == other.m_eUsage && m_eDimension == other.m_eDimension && m_eSampleCount == other.m_eSampleCount;
  }

  /// \brief Compares this instance with the specified \a other instance for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spTextureDescription& other) const { return !(*this == other); }

  /// \brief The width of the texture.
  ezUInt32 m_uiWidth;

  /// \brief The height of the texture.
  ezUInt32 m_uiHeight;

  /// \brief The depth of the texture.
  ezUInt32 m_uiDepth;

  /// \brief The number of mipmaps in the texture.
  ezUInt32 m_uiMipCount;

  /// \brief The number of array layers in the texture.
  ezUInt32 m_uiArrayLayers;

  /// \brief The format of each individual texture element.
  ezEnum<spPixelFormat> m_eFormat;

  /// \brief Controls how the texture is used.
  ezBitflags<spTextureUsage> m_eUsage;

  /// \brief The texture dimension (1D, 2D, or 3D).
  ezEnum<spTextureDimension> m_eDimension;

  /// \brief The number of samples in the texture.
  ezEnum<spTextureSampleCount> m_eSampleCount;
};

/// \brief Describes a \see spTextureView resource, for creation with a \see spDeviceResourceFactory.
struct spTextureViewDescription : public ezHashableStruct<spTextureViewDescription>
{
  /// \brief Creates a new empty \see spTextureViewDescription.
  spTextureViewDescription();

  /// \brief Creates a new \see spTextureViewDescription from an existing \see spTexture resource.
  /// \param pTexture The texture resource to create a new \see spTextureViewDescription from.
  spTextureViewDescription(const spTexture* pTexture);

  /// \brief Creates a new \see spTextureViewDescription from an existing \see spTexture resource.
  /// \note This constructor will override the pixel format of the target texture.
  /// \param pTexture The texture resource to create a new \see spTextureViewDescription from.
  /// \param eFormat The pixel format to use in the view. Should be compatible with the format of the target texture.
  spTextureViewDescription(const spTexture* pTexture, ezEnum<spPixelFormat> eFormat);

  /// \brief Compares this instance with the \a other instance for equality.
  EZ_ALWAYS_INLINE bool operator==(const spTextureViewDescription& other) const
  {
    return m_hTarget == other.m_hTarget && m_uiBaseMipLevel == other.m_uiBaseMipLevel && m_uiMipCount == other.m_uiMipCount && m_uiBaseArrayLayer == other.m_uiBaseArrayLayer && m_uiArrayLayers == other.m_uiArrayLayers && m_eFormat == other.m_eFormat;
  }

  /// \brief Compares this instance with the \a other instance for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spTextureViewDescription& other) const
  {
    return !(*this == other);
  }

  /// \brief The target \see spTexture resource.
  spResourceHandle m_hTarget{};

  /// \brief The base mip level visible from the view. Must be less than the number of mipmaps
  /// in the target texture.
  ezUInt32 m_uiBaseMipLevel{0};

  /// \brief The number of mip levels in the view.
  ezUInt32 m_uiMipCount{1};

  /// \brief The base array layer visible from the view. Must be less than the number of
  /// array layers in the target texture.
  ezUInt32 m_uiBaseArrayLayer{0};

  /// \brief The number of array layers in the view.
  ezUInt32 m_uiArrayLayers{0};

  /// \brief Specifies if the view should use a custom pixel format specified by \see spTextureViewDescription::m_eFormat.
  /// \note Set this value to false to use the pixel format of the target texture.
  bool m_bOverridePixelFormat{false};

  /// \brief The pixel format of the view. When \see spTextureViewDescription::m_bOverridePixelFormat is set to true,
  /// this value must store a pixel format "compatible" with the one of the target texture.
  ///
  /// For uncompressed formats, the overall size and number of components in this format must be equal to the ones of
  /// the target texture's pixel format. For compressed formats, it is only possible to use the same pixel format or
  /// its srgb/non-srgb pixel format counterpart.
  ezEnum<spPixelFormat> m_eFormat;
};

/// \brief Manages the state of textures resources in a graphics device.
class SP_RHI_DLL spTextureSamplerManager
{
  friend class spGraphicsDevice;

public:
  virtual ~spTextureSamplerManager() = default;

  /// \brief Gets a texture view resource for the specified texture resource.
  /// \param [in] hTexture A handle to the texture resource to retrieve the texture view resource from.
  /// \returns A handle to the texture view resource.
  EZ_NODISCARD virtual spResourceHandle GetFullTextureView(const spResourceHandle& hTexture) = 0;

protected:
  /// \brief Creates a texture/sampler manager for the given graphics device.
  /// \param [in] pDevice A pointer to the graphics device.
  spTextureSamplerManager(spGraphicsDevice* pDevice);
};

/// \brief A texture resource.
class SP_RHI_DLL spTexture : public spMappableResource
{
  friend class spDeviceResourceManager;

public:
  /// \brief Gets the format of individual texture elements stored in this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezEnum<spPixelFormat> GetFormat() const { return m_Description.m_eFormat; }

  /// \brief Gets the total width of this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetWidth() const { return m_Description.m_uiWidth; }

  /// \brief Gets the total height of this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetHeight() const { return m_Description.m_uiHeight; }

  /// \brief Gets the total depth of this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetDepth() const { return m_Description.m_uiDepth; }

  /// \brief Gets the total number of mipmaps of this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetMipCount() const { return m_Description.m_uiMipCount; }

  /// \brief Gets the total number of array slices of this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetArrayLayerCount() const { return m_Description.m_uiArrayLayers; }

  /// \brief Gets the usage flags given when this instance was created. This property controls how
  /// this instance is permitted to be used, and it is an error to attempt to use the texture outside of those contexts.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezBitflags<spTextureUsage> GetUsage() const { return m_Description.m_eUsage; }

  /// \brief Gets the dimensions of this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezEnum<spTextureDimension> GetDimension() const { return m_Description.m_eDimension; }

  /// \brief Gets the number os samples in this instance. If this returns any value other than
  /// \see spTextureSampleCount::None, the instance is considered a multisampled texture.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezEnum<spTextureSampleCount> GetSampleCount() const { return m_Description.m_eSampleCount; }

protected:
  explicit spTexture(spTextureDescription description)
    : m_Description(std::move(description))
  {
  }

  spTextureDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spTexture);

/// \brief A texture view resource.
class SP_RHI_DLL spTextureView : public spMappableResource
{
  friend class spDeviceResourceManager;

public:
  /// \brief Gets the target \see spTexture resource to be sampled via this instance.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual spResourceHandle GetTexture() const { return m_Description.m_hTarget; }

  /// \brief Gets the base mip level visible in the view.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetBaseMipLevel() const { return m_Description.m_uiBaseMipLevel; }

  /// \brief Gets the number of mipmaps in the view.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetMipCount() const { return m_Description.m_uiMipCount; }

  /// \brief Gets the base array layer visible in the view.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetBaseArrayLayer() const { return m_Description.m_uiBaseArrayLayer; }

  /// \brief Gets the number of array layers in the view.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetArrayLayerCount() const { return m_Description.m_uiArrayLayers; }

  /// \brief Gets the format used to interpret the contents of the target texture.
  ///
  /// \note This may be different than the format specified in the target texture, but
  /// it should be of the same size.
  EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezEnum<spPixelFormat> GetFormat() const { return m_Description.m_eFormat; }

protected:
  explicit spTextureView(spTextureViewDescription description)
    : m_Description(std::move(description))
  {
  }

  spTextureViewDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spTextureView);

class SP_RHI_DLL spPixelFormatHelper
{
public:
  EZ_ALWAYS_INLINE static ezUInt32 GetElementCount(const ezEnum<spInputElementFormat>& eFormat)
  {
    switch (eFormat)
    {
      case spInputElementFormat::Float1:
      case spInputElementFormat::UInt1:
      case spInputElementFormat::Int1:
      case spInputElementFormat::Half1:
        return 1;

      case spInputElementFormat::Float2:
      case spInputElementFormat::Byte2Norm:
      case spInputElementFormat::Byte2:
      case spInputElementFormat::SByte2Norm:
      case spInputElementFormat::SByte2:
      case spInputElementFormat::UShort2Norm:
      case spInputElementFormat::UShort2:
      case spInputElementFormat::Short2Norm:
      case spInputElementFormat::Short2:
      case spInputElementFormat::UInt2:
      case spInputElementFormat::Int2:
      case spInputElementFormat::Half2:
        return 2;

      case spInputElementFormat::Float3:
      case spInputElementFormat::UInt3:
      case spInputElementFormat::Int3:
        return 3;

      case spInputElementFormat::Float4:
      case spInputElementFormat::Byte4Norm:
      case spInputElementFormat::Byte4:
      case spInputElementFormat::SByte4Norm:
      case spInputElementFormat::SByte4:
      case spInputElementFormat::UShort4Norm:
      case spInputElementFormat::UShort4:
      case spInputElementFormat::Short4Norm:
      case spInputElementFormat::Short4:
      case spInputElementFormat::UInt4:
      case spInputElementFormat::Int4:
      case spInputElementFormat::Half4:
        return 4;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return 0;
    }
  }

  EZ_ALWAYS_INLINE static bool IsStencilFormat(const ezEnum<spPixelFormat>& eFormat)
  {
    return eFormat == spPixelFormat::D24UNormS8UInt || eFormat == spPixelFormat::D32FloatS8UInt;
  }

  EZ_ALWAYS_INLINE static bool IsDepthStencilFormat(const ezEnum<spPixelFormat>& eFormat)
  {
    return eFormat == spPixelFormat::D24UNormS8UInt || eFormat == spPixelFormat::D32FloatS8UInt || eFormat == spPixelFormat::D16UNorm || eFormat == spPixelFormat::D32Float;
  }

  EZ_ALWAYS_INLINE static bool IsCompressedFormat(const ezEnum<spPixelFormat>& eFormat)
  {
    return eFormat == spPixelFormat::Bc1RgbUNorm || eFormat == spPixelFormat::Bc1RgbUNormSRgb || eFormat == spPixelFormat::Bc1RgbaUNorm || eFormat == spPixelFormat::Bc1RgbaUNormSRgb || eFormat == spPixelFormat::Bc2UNorm || eFormat == spPixelFormat::Bc2UNormSRgb || eFormat == spPixelFormat::Bc3UNorm || eFormat == spPixelFormat::Bc3UNormSRgb || eFormat == spPixelFormat::Bc4UNorm || eFormat == spPixelFormat::Bc4SNorm || eFormat == spPixelFormat::Bc5UNorm || eFormat == spPixelFormat::Bc5SNorm || eFormat == spPixelFormat::Bc7UNorm || eFormat == spPixelFormat::Bc7UNormSRgb || eFormat == spPixelFormat::Etc2R8G8B8UNorm || eFormat == spPixelFormat::Etc2R8G8B8A1UNorm || eFormat == spPixelFormat::Etc2R8G8B8A8UNorm;
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetRowPitch(ezUInt32 uiWidth, const ezEnum<spPixelFormat>& eFormat)
  {
    if (IsCompressedFormat(eFormat))
    {
      ezUInt32 uiBlocksPerRow = (uiWidth + 3) / 4;
      ezUInt32 uiBlockSizeInBytes = GetBlockSizeInBytes(eFormat);
      return uiBlocksPerRow * uiBlockSizeInBytes;
    }

    return uiWidth * GetSizeInBytes(eFormat);
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetBlockSizeInBytes(const ezEnum<spPixelFormat>& eFormat)
  {
    switch (eFormat)
    {
      case spPixelFormat::Bc1RgbUNorm:
      case spPixelFormat::Bc1RgbUNormSRgb:
      case spPixelFormat::Bc1RgbaUNorm:
      case spPixelFormat::Bc1RgbaUNormSRgb:
      case spPixelFormat::Bc4UNorm:
      case spPixelFormat::Bc4SNorm:
      case spPixelFormat::Etc2R8G8B8UNorm:
      case spPixelFormat::Etc2R8G8B8A1UNorm:
        return 8;

      case spPixelFormat::Bc2UNorm:
      case spPixelFormat::Bc2UNormSRgb:
      case spPixelFormat::Bc3UNorm:
      case spPixelFormat::Bc3UNormSRgb:
      case spPixelFormat::Bc5UNorm:
      case spPixelFormat::Bc5SNorm:
      case spPixelFormat::Bc7UNorm:
      case spPixelFormat::Bc7UNormSRgb:
      case spPixelFormat::Etc2R8G8B8A8UNorm:
        return 16;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return 0;
    }
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetSizeInBytes(const ezEnum<spPixelFormat>& eFormat)
  {
    switch (eFormat)
    {
      case spPixelFormat::R8UNorm:
      case spPixelFormat::R8SNorm:
      case spPixelFormat::R8UInt:
      case spPixelFormat::R8SInt:
        return 1;

      case spPixelFormat::R16UNorm:
      case spPixelFormat::R16SNorm:
      case spPixelFormat::R16UInt:
      case spPixelFormat::R16SInt:
      case spPixelFormat::R16Float:
      case spPixelFormat::R8G8UNorm:
      case spPixelFormat::R8G8SNorm:
      case spPixelFormat::R8G8UInt:
      case spPixelFormat::R8G8SInt:
        return 2;

      case spPixelFormat::R32UInt:
      case spPixelFormat::R32SInt:
      case spPixelFormat::R32Float:
      case spPixelFormat::R16G16UNorm:
      case spPixelFormat::R16G16SNorm:
      case spPixelFormat::R16G16UInt:
      case spPixelFormat::R16G16SInt:
      case spPixelFormat::R16G16Float:
      case spPixelFormat::R8G8B8A8UNorm:
      case spPixelFormat::R8G8B8A8UNormSRgb:
      case spPixelFormat::R8G8B8A8SNorm:
      case spPixelFormat::R8G8B8A8UInt:
      case spPixelFormat::R8G8B8A8SInt:
      case spPixelFormat::B8G8R8A8UNorm:
      case spPixelFormat::B8G8R8A8UNormSRgb:
      case spPixelFormat::R10G10B10A2UNorm:
      case spPixelFormat::R10G10B10A2UInt:
      case spPixelFormat::R11G11B10Float:
      case spPixelFormat::D24UNormS8UInt:
        return 4;

      case spPixelFormat::D32FloatS8UInt:
        return 5;

      case spPixelFormat::R16G16B16A16UNorm:
      case spPixelFormat::R16G16B16A16SNorm:
      case spPixelFormat::R16G16B16A16UInt:
      case spPixelFormat::R16G16B16A16SInt:
      case spPixelFormat::R16G16B16A16Float:
      case spPixelFormat::R32G32UInt:
      case spPixelFormat::R32G32SInt:
      case spPixelFormat::R32G32Float:
        return 8;

      case spPixelFormat::R32G32B32A32Float:
      case spPixelFormat::R32G32B32A32UInt:
      case spPixelFormat::R32G32B32A32SInt:
        return 16;

      case spPixelFormat::Bc1RgbUNorm:
      case spPixelFormat::Bc1RgbUNormSRgb:
      case spPixelFormat::Bc1RgbaUNorm:
      case spPixelFormat::Bc1RgbaUNormSRgb:
      case spPixelFormat::Bc2UNorm:
      case spPixelFormat::Bc2UNormSRgb:
      case spPixelFormat::Bc3UNorm:
      case spPixelFormat::Bc3UNormSRgb:
      case spPixelFormat::Bc4UNorm:
      case spPixelFormat::Bc4SNorm:
      case spPixelFormat::Bc5UNorm:
      case spPixelFormat::Bc5SNorm:
      case spPixelFormat::Bc7UNorm:
      case spPixelFormat::Bc7UNormSRgb:
      case spPixelFormat::Etc2R8G8B8UNorm:
      case spPixelFormat::Etc2R8G8B8A1UNorm:
      case spPixelFormat::Etc2R8G8B8A8UNorm:
        EZ_ASSERT_DEV(false, "GetSizeInBytes should not be used on a compressed format.");
        return 0;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return 0;
    }
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetSizeInBytes(const ezEnum<spInputElementFormat>& eFormat)
  {
    switch (eFormat)
    {
      case spInputElementFormat::Byte2:
      case spInputElementFormat::Byte2Norm:
      case spInputElementFormat::SByte2Norm:
      case spInputElementFormat::SByte2:
      case spInputElementFormat::Half1:
        return 2;

      case spInputElementFormat::Float1:
      case spInputElementFormat::UInt1:
      case spInputElementFormat::Int1:
      case spInputElementFormat::Byte4:
      case spInputElementFormat::Byte4Norm:
      case spInputElementFormat::SByte4:
      case spInputElementFormat::SByte4Norm:
      case spInputElementFormat::UShort2:
      case spInputElementFormat::UShort2Norm:
      case spInputElementFormat::Short2:
      case spInputElementFormat::Short2Norm:
      case spInputElementFormat::Half2:
        return 4;

      case spInputElementFormat::Float2:
      case spInputElementFormat::UInt2:
      case spInputElementFormat::Int2:
      case spInputElementFormat::UShort4:
      case spInputElementFormat::UShort4Norm:
      case spInputElementFormat::Short4:
      case spInputElementFormat::Short4Norm:
      case spInputElementFormat::Half4:
        return 8;

      case spInputElementFormat::Float3:
      case spInputElementFormat::UInt3:
      case spInputElementFormat::Int3:
        return 12;

      case spInputElementFormat::Float4:
      case spInputElementFormat::UInt4:
      case spInputElementFormat::Int4:
        return 16;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return 0;
    }
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetNumRows(ezUInt32 uiHeight, const ezEnum<spPixelFormat>& eFormat)
  {
    return IsCompressedFormat(eFormat) ? (uiHeight + 3) / 4 : uiHeight;
  }

  EZ_ALWAYS_INLINE static ezUInt32 GetDepthPitch(ezUInt32 uiRowPitch, ezUInt32 uiHeight, const ezEnum<spPixelFormat>& eFormat)
  {
    return uiRowPitch * GetNumRows(uiHeight, eFormat);
  }
};

class SP_RHI_DLL spTextureHelper
{
public:
  /// \brief Calculates the number of mipmaps in a texture.
  /// \param uiWidth The width of the texture.
  /// \param uiHeight The height of the texture.
  /// \return The number of mipmaps in a texture with the given \a uiWidth and \a uiHeight.
  EZ_ALWAYS_INLINE static ezUInt32 CalculateMipCount(ezUInt32 uiWidth, ezUInt32 uiHeight)
  {
    return 1 + static_cast<ezUInt32>(ezMath::Floor(ezMath::Log2(ezMath::Max(static_cast<float>(uiWidth), static_cast<float>(uiHeight)))));
  }

  /// \brief Calculates the subresource index, given a mipmap level and an array layer.
  /// \param pTexture The texture to calculate the subresource index from.
  /// \param uiMipLevel The mipmap level to calculate the subresource index for.
  /// \param uiArrayLayer The array layer to calculate the subresource index for.
  /// \return The subresource index.
  EZ_ALWAYS_INLINE static ezUInt32 CalculateSubresource(const spTexture* pTexture, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer)
  {
    return uiArrayLayer * pTexture->GetMipCount() + uiMipLevel;
  }

  /// \brief Gets the mip level and the array layer of a texture given its subresource index.
  /// \param [in] pTexture The texture to get the subresource from.
  /// \param [in] uiSubresource The subresource index to get the mip level and the array layer of the texture.
  /// \param [out] uiMipLevel The mip level of the texture at the given subresource index.
  /// \param [out] uiArrayLayer The array layer of the texture at the given subresource index.
  EZ_ALWAYS_INLINE static void GetMipLevelAndArrayLayer(const spTexture* pTexture, ezUInt32 uiSubresource, ezUInt32& uiMipLevel, ezUInt32& uiArrayLayer)
  {
    uiArrayLayer = uiSubresource / pTexture->GetMipCount();
    uiMipLevel = uiSubresource - uiArrayLayer * pTexture->GetMipCount();
  }

  /// \brief Gets the dimensions of a mipmap level from a texture.
  /// \param [in] pTexture The texture to get the dimensions from.
  /// \param [in] uiMipLevel The mipmap level to get the dimensions from.
  /// \param [out] uiWidth The width of the mipmap level.
  /// \param [out] uiHeight The height of the mipmap level.
  /// \param [out] uiDepth The depth of the mipmap level.
  EZ_ALWAYS_INLINE static void GetMipDimensions(const spTexture* pTexture, ezUInt32 uiMipLevel, ezUInt32& uiWidth, ezUInt32& uiHeight, ezUInt32& uiDepth)
  {
    uiWidth = GetMipDimension(pTexture->GetWidth(), uiMipLevel);
    uiHeight = GetMipDimension(pTexture->GetHeight(), uiMipLevel);
    uiDepth = GetMipDimension(pTexture->GetDepth(), uiMipLevel);
  }

  /// \brief Gets the dimension of a mipmap level from a texture given the largest dimension.
  /// \param uiLargestDimension The largest dimension.
  /// \param uiMipLevel The mipmap level to get the dimension from.
  /// \return The dimension of the mipmap level.
  EZ_ALWAYS_INLINE static ezUInt32 GetMipDimension(ezUInt32 uiLargestMipDimension, ezUInt32 uiMipLevel)
  {
    ezUInt32 uiDim = uiLargestMipDimension;
    for (ezUInt32 i = 0; i < uiMipLevel; i++)
      uiDim /= 2;

    return ezMath::Max<ezUInt32>(uiDim, 1);
  }

  EZ_ALWAYS_INLINE static void CopyTextureRegion(
    const void* pSource,
    ezUInt32 uiSourceX,
    ezUInt32 uiSourceY,
    ezUInt32 uiSourceZ,
    ezUInt32 uiSourceRowPitch,
    ezUInt32 uiSourceDepthPitch,
    void* pDestination,
    ezUInt32 uiDestinationX,
    ezUInt32 uiDestinationY,
    ezUInt32 uiDestinationZ,
    ezUInt32 uiDestinationRowPitch,
    ezUInt32 uiDestinationDepthPitch,
    ezUInt32 uiWidth,
    ezUInt32 uiHeight,
    ezUInt32 uiDepth,
    const ezEnum<spPixelFormat>& eFormat)
  {
    ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(eFormat) ? 4 : 1;
    ezUInt32 uiBlockSizeInBytes = uiBlockSize > 1 ? spPixelFormatHelper::GetBlockSizeInBytes(eFormat) : spPixelFormatHelper::GetSizeInBytes(eFormat);
    ezUInt32 uiCompressedSourceX = uiSourceX / uiBlockSize;
    ezUInt32 uiCompressedSourceY = uiSourceY / uiBlockSize;
    ezUInt32 uiCompressedDestinationX = uiDestinationX / uiBlockSize;
    ezUInt32 uiCompressedDestinationY = uiDestinationY / uiBlockSize;
    ezUInt32 uiNumRows = spPixelFormatHelper::GetNumRows(uiHeight, eFormat);
    ezUInt32 uiRowSize = uiWidth / uiBlockSize * uiBlockSizeInBytes;

    if (uiSourceRowPitch == uiDestinationRowPitch && uiSourceDepthPitch == uiDestinationDepthPitch)
    {
      ezUInt32 uiTotalCopySize = uiDepth * uiSourceDepthPitch;
      ezMemoryUtils::RawByteCopy(pDestination, pSource, uiTotalCopySize);
    }
    else
    {
      for (ezUInt32 zz = 0; zz < uiDepth; zz++)
      {
        for (ezUInt32 yy = 0; yy < uiNumRows; yy++)
        {
          ezUInt8* pRowCopyDst = reinterpret_cast<ezUInt8*>(pDestination) + uiDestinationDepthPitch * (zz + uiDestinationZ) + uiDestinationRowPitch * (yy + uiCompressedDestinationY) + uiBlockSizeInBytes * uiCompressedSourceX;
          const ezUInt8* pRowCopySrc = reinterpret_cast<const ezUInt8*>(pSource) + uiSourceDepthPitch * (zz + uiSourceZ) + uiSourceRowPitch * (yy + uiCompressedSourceY) + uiBlockSizeInBytes * uiCompressedDestinationX;
          ezMemoryUtils::Copy(pRowCopyDst, pRowCopySrc, uiRowSize);
        }
      }
    }
  }
};
