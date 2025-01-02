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

implementing Common;

public struct PerFrameBuffer
{
  public float GlobalTime;
  public float CurrentTime;
  public float DeltaTime;
  public uint  Frame;

  public float2 RenderSize;
  public float2 InverseRenderSize;

  public float2 ViewportSize;
  public float2 InverseViewportSize;

  public uint  Options;
  public bool  HDREnabled;
  public float HDRMaxNits;
  public float HDRWhitePoint;
}

public struct PerViewBuffer
{
  public float4 Position_NearClipPlane;

  public float4 PreviousPosition_FarClipPlane;

  public float4 Direction_ShutterSpeed;

  public float Exposure;
  public float Aperture;
  public float ISO;
  public float AspectRatio;

  public matrix ProjectionMatrix;
  public matrix InverseProjectionMatrix;

  public matrix ViewMatrix;
  public matrix InverseViewMatrix;

  public matrix ViewProjectionMatrix;
  public matrix InverseViewProjectionMatrix;

  public matrix PreviousViewProjectionMatrix;
  public matrix PreviousInverseViewProjectionMatrix;

  public property Position : float3
  {
    get {
      return Position_NearClipPlane.xyz;
    }
  }

  public property NearClipPlane : float
  {
    get { return Position_NearClipPlane.w; }
  }

  public property PreviousPosition : float3
  {
    get {
      return PreviousPosition_FarClipPlane.xyz;
    }
  }

  public property FarClipPlane : float
  {
    get { return PreviousPosition_FarClipPlane.w; }
  }

  public property Direction : float3
  {
    get {
      return Direction_ShutterSpeed.xyz;
    }
  }

  public property ShutterSpeed : float
  {
    get { return Direction_ShutterSpeed.w; }
  }
}

/// 128-bit push constant buffer
public struct PerPassBuffer
{
  public float4 Values0;
  public float4 Values1;
  public float4 Values2;
  public float4 Values3;
  public float4 Values4;
  public float4 Values5;
  public float4 Values6;
  public float4 Values7;
};

// [[push_constant]]
// PerPassBuffer Buffer_PerPass : register(b0, space0); // Push constants have the requirement to be at binding 0.

// /// Checks if the current rendering object is transparent.
// func IsTransparent() -> bool { return Buffer_PerPass.Values._m13 == 1.0f; }

// /// Checks if the current rendering object is opaque.
// func IsOpaque() -> bool { return !IsTransparent(); }
