// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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
// limitations under  the License.

#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/Device.h>
#include <RPI/Scene/SceneContext.h>

namespace RPI
{
  class spRenderObject;
  class spRenderObjectCollection;
  class spRenderSystem;
  class spRenderView;

  /// \brief Stores a collection of \a spRenderObject visible within a \a spRenderView.
  class SP_RPI_DLL spVisibilityGroup
  {
    friend class spRenderObjectCollection;

  public:
    explicit spVisibilityGroup(spSceneContext* pSceneContext);
    ~spVisibilityGroup();

    void Reset();

    void Extract(spRenderView* pView);

    void Copy(spRenderView* pSource, spRenderView* pDestination);

    spRenderObjectCollection* GetRenderObjectCollection() { return &m_RenderObjects; }

  private:
    static void EvaluateActiveRenderStages(spRenderObject* pRenderObject);

    void AddRenderObject(spRenderObjectCollection* pCollection, spRenderObject* pRenderObject);
    void RemoveRenderObject(spRenderObjectCollection* pCollection, spRenderObject* pRenderObject);

    void EvaluateActiveRenderStages();

    bool m_bNeedRenderStageEvaluation{false};
    ezDynamicArray<spRenderObject*> m_RenderObjectsWithoutFeatures{nullptr};
    spRenderObjectCollection m_RenderObjects{nullptr};

    spSceneContext* m_pSceneContext{nullptr};
  };
} // namespace RPI
