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

#include <RPI/Features/RenderFeatureExtractor.h>
#include <RPI/Meshes/MeshRenderObject.h>

#include <RAI/Resources/MeshResource.h>

#include <Foundation/Communication/Message.h>

namespace RPI
{
  /// \brief A message to extract a \a spMeshRenderObject from a game object.
  struct SP_RPI_DLL spExtractMeshRenderObjectMessage : public ezMessage
  {
    EZ_DECLARE_MESSAGE_TYPE(spExtractMeshRenderObjectMessage, ezMessage);

    /// \brief The \a spRenderContext in which data are being extracted.
    const spRenderContext* m_pRenderContext{nullptr};

    /// \brief The \a spRenderObjectCollection in which mesh render objects are filled.
    spRenderObjectCollection* m_Objects{nullptr};
  };

  /// \brief An extractor implementation to collect mesh objects from game objects
  class SP_RPI_DLL spMeshRenderFeatureExtractor final : public spRenderFeatureExtractor
  {
    friend class spMeshRenderFeature;

    EZ_ADD_DYNAMIC_REFLECTION(spMeshRenderFeatureExtractor, spRenderFeatureExtractor);

    // spRenderFeatureExtractor

  public:
    void Extract(spSceneContext* pSceneContext, const spRenderContext* pRenderContext) override;
  };
} // namespace RPI
