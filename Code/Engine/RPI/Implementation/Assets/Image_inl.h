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

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spImage& image)
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

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spImage& ref_image)
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
