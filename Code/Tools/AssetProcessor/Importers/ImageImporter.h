#pragma once

#include <AssetProcessor/Importers/Importer.h>

struct spTextureImageType
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    Color,
    Greyscale,
    NormalMap,
    HeightMap,
    Material,
    HDR,
    RawData,

    Default = Color
  };
};

struct spTextureCompressionLevel
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    None,
    Medium,
    High,

    Default = None
  };
};

struct spTextureMipmapFilter
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    Kaiser,
    Triangle,
    Box,

    Default = Kaiser
  };
};

struct spTextureWrapMode
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    Clamp,
    Repeat,
    Mirror,

    Default = Clamp
  };
};

struct spImageImporterConfiguration
{
  bool m_bFlipHorizontal{false};

  bool m_bFlipVertical{false};

  bool m_bPremultiplyAlpha{false};

  bool m_bGenerateMipMaps{false};

  /// \brief The number of mip levels to generate if \a m_bGenerateMipMaps is true.
  /// Set to 0 to generate all mip levels.
  ezUInt32 m_uiNumMipLevels{0};

  ezEnum<spTextureMipmapFilter> m_eMipmapFilter{spTextureMipmapFilter::Default};

  ezEnum<spTextureWrapMode> m_eWrapMode{spTextureWrapMode::Default};

  bool m_bPreserveAlphaCoverage{false};

  float m_fAlphaCoverageThreshold{0.0f};

  ezEnum<spTextureImageType> m_eType{spTextureImageType::Default};

  ezEnum<spTextureCompressionLevel> m_eCompressionLevel{spTextureCompressionLevel::Default};
};

class spImage2DImporter final : public spImporter<spImageImporterConfiguration>
{
public:
  explicit spImage2DImporter(const spImageImporterConfiguration& configuration);
  ~spImage2DImporter() override = default;

  ezResult Import(ezStringView sAssetPath, ezStringView sOutputPath) override;

private:
  // ezByteBlobPtr CompressBC3(const ezImageView& source);
};
