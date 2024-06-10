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

/// Enables a feature checked with \a SP_ENABLED
#define SP_ON 1

/// Disables a feature checked with \a SP_ENABLED
#define SP_OFF 0

/// Checks if the specified feature is enabled
#define SP_ENABLED(_MACRO_) _MACRO_ == SP_ON

/// Checks if the specified feature is disabled
#define SP_DISABLED(_MACRO_) _MACRO_ == SP_OFF

/// The OpenGL RHI backend
#define SP_RHI_OPENGL 0

/// The OpenGLES RHI backend
#define SP_RHI_OPENGLES 1

/// The D3D11 RHI backend
#define SP_RHI_D3D11 2

/// The D3D12 RHI backend
#define SP_RHI_D3D12 3

/// The Vulkan RHI backend
#define SP_RHI_VULKAN 4

/// The Metal RHI backend
#define SP_RHI_METAL 5

// Disable vertex skinnig by default
#ifndef SP_FEATURE_VERTEX_SKINNING
#  define SP_FEATURE_VERTEX_SKINNING SP_OFF
#endif
