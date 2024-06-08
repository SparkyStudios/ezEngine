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
  float4 Position_NearClipPlane;

  float4 PreviousPosition_FarClipPlane;

  float4 Direction_ShutterSpeed;

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

  property Position: float3
  {
    get { return Position_NearClipPlane.xyz; }
  }

  property NearClipPlane: float
  {
    get { return Position_NearClipPlane.w; }
  }

  property PreviousPosition: float3
  {
    get { return PreviousPosition_FarClipPlane.xyz; }
  }

  property FarClipPlane: float
  {
    get { return PreviousPosition_FarClipPlane.w; }
  }

  property Direction: float3
  {
    get { return Direction_ShutterSpeed.xyz; }
  }

  property ShutterSpeed: float
  {
    get { return Direction_ShutterSpeed.w; }
  }
}

/// 128-bit push constant buffer
struct PerPassBuffer
{
  float4 Values0;
  float4 Values1;
  float4 Values2;
  float4 Values3;
  float4 Values4;
  float4 Values5;
  float4 Values6;
  float4 Values7;
};

// [[push_constant]]
// PerPassBuffer Buffer_PerPass : register(b0, space0); // Push constants have the requirement to be at binding 0.

// /// Checks if the current rendering object is transparent.
// func IsTransparent() -> bool { return Buffer_PerPass.Values._m13 == 1.0f; }

// /// Checks if the current rendering object is opaque.
// func IsOpaque() -> bool { return !IsTransparent(); }
