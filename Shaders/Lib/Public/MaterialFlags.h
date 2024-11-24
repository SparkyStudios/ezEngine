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

#ifndef SP_RHI_SHADER
#  define uint ezUInt32
#endif

static constexpr uint k_HasTextureAlbedoMask    = 0;
static constexpr uint k_HasTextureNormalMask    = 1;
static constexpr uint k_HasTextureMetalnessMask = 2;
static constexpr uint k_HasTextureRoughnessMask = 3;
static constexpr uint k_HasTextureOcclusionMask = 4;
static constexpr uint k_HasTextureCavityMask    = 5;
static constexpr uint k_HasTextureORMCMask      = 6;
static constexpr uint k_HasTextureSpecularMask  = 7;
static constexpr uint k_HasTextureEmissiveMask  = 8;
static constexpr uint k_HasTextureAlphaMask     = 9;
static constexpr uint k_HasTextureHeightMask    = 10;

/// The maximum value for reserved flag masks. You should use this value
/// for your custom flags, by adding at least one.
/// Example:
///```slang
/// static constexpr uint k_MyCustomMaterialMask1 = k_MaxReservedMaterialMask + 1;
/// static constexpr uint k_MyCustomMaterialMask2 = k_MaxReservedMaterialMask + 2;
/// ```
static constexpr uint k_MaxReservedMaterialMask = 10;

#ifndef SP_RHI_SHADER
#  undef uint
#endif
