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

implementing MaterialSystem;

/// A material interface describing all the necessary methods used to generate
/// surface data for a geometry.
///
/// Its the responsibility of the material implementation to ensure valid data are returned.
public interface IMaterial
{
  /// Creates a new default instance of the material.
  static This create();

  /// Gets the normal vector.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's normal vector.
  float3 GetNormal(in TransientStream vertex, in MaterialData data);

  /// Gets the diffuse color.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's diffuse color.
  float3 GetDiffuseColor(in TransientStream vertex, in MaterialData data);

  /// Gets the specular color.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's specular color.
  float3 GetSpecularColor(in TransientStream vertex, in MaterialData data);

  /// Gets the emissive color.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's emissive color.
  float3 GetEmissiveColor(in TransientStream vertex, in MaterialData data);

  /// Gets the linear roughness.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's linear roughness.
  float GetLinearRoughness(in TransientStream vertex, in MaterialData data);

  /// Gets the opacity.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's opacity.
  float GetOpacity(in TransientStream vertex, in MaterialData data);

  /// Gets the ambient occlusion.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's ambient occlusion.
  float GetOcclusion(in TransientStream vertex, in MaterialData data);

  /// Gets the cavity factor.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's cavity.
  float GetCavity(in TransientStream vertex, in MaterialData data);

  /// Get the metalness.
  /// \param vertex The output of the vertex shader.
  /// \param data The geometry's material data.
  /// \return The surface's metalness.
  float GetMetalness(in TransientStream vertex, in MaterialData data);
}
