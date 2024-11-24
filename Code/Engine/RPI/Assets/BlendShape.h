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

#include <RPI/RPIDLL.h>

#include <RPI/Core.h>

namespace RPI
{
  /// \brief A blend shape asset.
  ///
  /// A blend shape is used when doing morph target animations. This asset
  /// stores the data for a single blend shape within a mesh. That blend shape
  /// may be compatible with other meshes other than the source mesh.
  class SP_RPI_DLL spBlendShape
  {
    friend class spBlendShapeResource;
    friend class spBlendShapeResourceDescriptor;

  public:
    /// \brief The blend shape name.
    ezHashedString m_sName;

    /// \brief The blend shape vertices.
    ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper> m_Vertices;
    ezUInt32 m_uiVertexSize;

    /// \brief The blend shape weight. This affects how much the vertices are blended together.
    float m_fWeight{0};
  };
} // namespace RPI

#include <RPI/Implementation/Assets/BlendShape_inl.h>
