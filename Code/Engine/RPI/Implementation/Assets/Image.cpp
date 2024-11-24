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

#include <RPI/RPIPCH.h>

#include <RPI/Assets/Image.h>

#include <ktx.h>

namespace RPI
{
  ezByteBlobPtr spImage::GetImageData(ezUInt32 uiMipLevel, ezUInt32 uiDepth, ezUInt32 uiArrayLayer) const
  {
    if (m_pKtxTexture == nullptr)
      return {};

    if (uiArrayLayer >= m_uiArrayLayers || uiDepth >= m_uiDepth || uiMipLevel >= m_uiMipCount)
      return {};

    ktx_size_t offset = 0;
    KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture(m_pKtxTexture), uiMipLevel, uiArrayLayer, uiDepth, &offset);

    if (result != KTX_SUCCESS)
      return {};

    return {ktxTexture_GetData(ktxTexture(m_pKtxTexture)) + offset, ktxTexture_GetImageSize(ktxTexture(m_pKtxTexture), uiMipLevel)};
  }

  ezResult spImage::LoadImageData()
  {
    if (m_pKtxTexture != nullptr)
      UnloadImageData();

    ktxTexture2* texture;
    KTX_error_code result;

    result = ktxTexture2_CreateFromMemory(m_Storage.GetData(), m_Storage.GetStorageSize64(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);

    if (result != KTX_SUCCESS)
      return EZ_FAILURE;

    m_pKtxTexture = texture;

    return EZ_SUCCESS;
  }

  void spImage::UnloadImageData()
  {
    ktxTexture_Destroy(ktxTexture(m_pKtxTexture));
    m_pKtxTexture = nullptr;
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Assets_Image);
