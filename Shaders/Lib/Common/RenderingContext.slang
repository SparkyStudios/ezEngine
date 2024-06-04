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

struct PerFrameBuffer
{
  float GlobalTime;
  float CurrentTime;
  float DeltaTime;
  uint Frame;

  float2 RenderSize;
  float2 InverseRenderSize;

  float2 ViewportSize;
  float2 InverseViewportSize;

  uint Options;
  bool HDREnabled;
  float HDRMaxNits;
  float HDRWhitePoint;
}

struct PerViewBuffer
{
  float3 Position;
  float NearClipPlane;

  float3 PreviousPosition;
  float FarClipPlane;

  float3 Direction;
  float ShutterSpeed;

  float Exposure;
  float Aperture;
  float ISO;
  float AspectRatio;

  matrix ProjectionMatrix;
  matrix InverseProjectionMatrix;

  matrix ViewMatrix;
  matrix InverseViewMatrix;

  matrix ViewProjectionMatrix;
  matrix InverseViewProjectionMatrix;
}

/// 128-bit push constant buffer
struct PerPassBuffer
{
  matrix Transform;
  matrix Values;
};

[[vk_binding(1, 0)]]
ConstantBuffer<PerFrameBuffer> Buffer_PerFrame : register(b1, space0);

[[vk_binding(2, 0)]]
ConstantBuffer<PerViewBuffer> Buffer_PerView : register(b2, space0);

[[push_constant]]
PerPassBuffer Buffer_PerPass : register(b0, space0); // Push constants have the requirement to be at binding 0.

/// Gets the index in the materials buffer for the current rendering object.
func GetMaterialIndex() -> uint { return uint(Buffer_PerPass.Values._m03); }

/// Checks if the current rendering object is transparent.
func IsTransparent() -> bool { return Buffer_PerPass.Values._m13 == 1.0f; }

/// Checks if the current rendering object is opaque.
func IsOpaque() -> bool { return !IsTransparent(); }
