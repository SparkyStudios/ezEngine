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

extern static const bool o_IsClipSpaceYInverted = false;

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
  uint HDREnabled;
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

/// \brief 128-bit push constant buffer
struct PerPassBuffer
{
  matrix Transform;
  matrix Values;
};

ConstantBuffer<PerFrameBuffer> Buffer_PerFrame;
ConstantBuffer<PerViewBuffer> Buffer_PerView : register(b0, space0);

[[push_constant]]
// TODO: The binding model used in the Metal backend forces me to set this buffer
// to binding 2 (the next available binding after the vertext buffer), because the vertex buffer
// uses the binding 1 (which may change over shaders). Not sure how to deal with this,
// will need more tests for other backends.
PerPassBuffer Buffer_PerPass : register(b2, space0);

/// \brief Gets the index in the materials buffer for the current rendering object.
uint GetMaterialIndex() { return /*uint(Buffer_PerPass.Values._m03)*/ 0; }

/// \brief Checks if the current rendering object is transparent.
bool IsTransparent() { return Buffer_PerPass.Values._m13 == 1.0f; }

/// \brief Checks if the current rendering object is opaque.
bool IsOpaque() { return !IsTransparent(); }
