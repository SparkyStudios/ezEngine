#pragma once

#include <Texture/Image/Formats/ImageFileFormat.h>

/// Png and jpeg file format support using stb_image.
///
/// stb_image also supports other formats, but we stick to our own loader code where we can.
/// Also, stb HDR image support is not handled here yet.
class EZ_TEXTURE_DLL ezStbImageFileFormats : public ezImageFileFormat
{
public:
  virtual ezResult ReadImageHeader(ezStreamReader& inout_stream, ezImageHeader& ref_header, ezStringView sFileExtension) const override;
  virtual ezResult ReadImage(ezStreamReader& inout_stream, ezImage& ref_image, ezStringView sFileExtension) const override;
  virtual ezResult WriteImage(ezStreamWriter& inout_stream, const ezImageView& image, ezStringView sFileExtension) const override;

  virtual bool CanReadFileType(ezStringView sExtension) const override;
  virtual bool CanWriteFileType(ezStringView sExtension) const override;
};
