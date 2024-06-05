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

#include <Materials/MaterialData.slang>

/// A material interface describing all the necessary methods used to generate
/// surface data for a geometry.
///
/// Its the responsibility of the material implementation to ensure valid data are returned.
interface IMaterial
{
  /// Gets the normal vector.
  /// \param data The geometry's material data.
  /// \return The surface's normal vector.
  func GetNormal(in data: MaterialData) -> float3;

  /// Gets the albedo color.
  /// \param data The geometry's material data.
  /// \return The surface's albedo color.
  func GetAlbedoColor(in data: MaterialData) -> float3;

  /// Gets the specular color.
  /// \param data The geometry's material data.
  /// \return The surface's specular color.
  func GetSpecularColor(in data: MaterialData) -> float3;

  /// Gets the emissive color.
  /// \param data The geometry's material data.
  /// \return The surface's emissive color.
  func GetEmissiveColor(in data: MaterialData) -> float3;

  /// Gets the linear roughness.
  /// \param data The geometry's material data.
  /// \return The surface's linear roughness.
  func GetLinearRoughness(in data: MaterialData) -> float;

  /// Gets the opacity.
  /// \param data The geometry's material data.
  /// \return The surface's opacity.
  func GetOpacity(in data: MaterialData) -> float;

  /// Gets the ambient occlusion.
  /// \param data The geometry's material data.
  /// \return The surface's ambient occlusion.
  func GetOcclusion(in data: MaterialData) -> float;

  /// Gets the cavity factor.
  /// \param data The geometry's material data.
  /// \return The surface's cavity.
  func GetCavity(in data: MaterialData) -> float;

  /// Get the metalness.
  /// \param data The geometry's material data.
  /// \return The surface's metalness.
  func GetMetalness(in data: MaterialData) -> float;
}
