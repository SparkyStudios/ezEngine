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

implementing Math;

/// Convenient type to store and share transform
/// data (translation, rotation, scale) between CPU and GPU.
public struct Transform
{
  public float4 r0;
  public float4 r1;
  public float4 r2;
}

/// Gets the transformation matrix from the given transform.
/// \param t The transform.
/// \return A 4x4 transformation matrix.
public matrix TransformToMatrix(Transform t) { return float4x4(t.r0, t.r1, t.r2, float4(0, 0, 0, 1)); }

/// Gets the position from the given transform.
/// \param t The transform.
/// \return The position as a \c float4 value. The W component of the vector is always 1.
public float4 TransformToPosition(Transform t) { return float4(t.r0.w, t.r1.w, t.r2.w, 1); }

/// Gets the rotation matrix from the given transform.
/// \param t The transform.
/// \return A 3x3 rotation matrix.
public matrix<float, 3, 3> TransformToRotation(Transform t) { return float3x3(t.r0.xyz, t.r1.xyz, t.r2.xyz); }
