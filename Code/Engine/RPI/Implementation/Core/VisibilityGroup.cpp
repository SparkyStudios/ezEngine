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

#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderObject.h>
#include <RPI/Core/VisibilityGroup.h>

namespace RPI
{
  spVisibilityGroup::spVisibilityGroup(spSceneContext* pSceneContext)
    : m_RenderObjectsWithoutFeatures()
    , m_RenderObjects(this)
    , m_pSceneContext(pSceneContext)
  {
  }

  spVisibilityGroup::~spVisibilityGroup()
  {
  }

  void spVisibilityGroup::Reset()
  {
    // Clear render objects
    for (auto& pObject : m_RenderObjects.m_Items)
      pObject->m_VisibilityGroupReference = spRenderNodeReference::MakeInvalid();

    m_RenderObjects.Clear();
  }

  void spVisibilityGroup::Extract(spRenderView* pView)
  {
    EZ_PROFILE_SCOPE("spVisibilityGroup::Collect");

    if (pView->m_uiLastCollectedFrame == spRenderSystem::GetFrameCount())
      return; // The view has already been collected for this frame

    pView->m_VisibleRenderObjects.Clear();
    pView->m_uiLastCollectedFrame = spRenderSystem::GetFrameCount();

    EvaluateActiveRenderStages();

    const ezFrustum frustum = pView->GetFrustum();

    ezTaskSystem::ParallelForIndexed(0, m_RenderObjects.m_Items.GetCount(), [&](ezUInt32 uiStartIndex, ezUInt32 uiEndIndex)
      {
      for (ezUInt32 i = uiStartIndex; i < uiEndIndex; ++i)
      {
        spRenderObject* pRenderObject = m_RenderObjects.m_Items[i];

        if (pView->GetCullingMode() == spRenderViewCullingMode::Frustum
          && pRenderObject->m_BoundingBox.IsValid()
          && frustum.GetObjectPosition(pRenderObject->m_BoundingBox) == ezVolumePosition::Outside)
        {
          return;
        }

        pView->m_VisibleRenderObjects.Add(pRenderObject);

        if (pRenderObject->m_BoundingBox.IsValid())
        {
          pView->m_BoundingBox.ExpandToInclude(pRenderObject->m_BoundingBox);
        }
      } });

    pView->m_VisibleRenderObjects.Close();
  }

  void spVisibilityGroup::Copy(spRenderView* pSource, spRenderView* pDestination)
  {
    // TODO
  }

  void spVisibilityGroup::EvaluateActiveRenderStages(spRenderObject* pRenderObject)
  {
    if (pRenderObject->m_pRenderFeature == nullptr)
      return;

    const auto* pRenderSystem = spRenderSystem::GetSingleton();

    const ezUInt32 uiStagesCount = pRenderSystem->GetRenderStageCount();

    if (uiStagesCount == 0)
      return;

    // Determine which render stages are active for this render object
    pRenderObject->m_ActiveRenderStages.SetCount(uiStagesCount);

    for (const auto& pRenderStage : pRenderSystem->GetRenderStages())
    {
      const ezInt32 iRef = pRenderStage->GetRenderSystemReference().GetRef();
      pRenderObject->m_ActiveRenderStages[iRef] = pRenderSystem->GetRenderStage(iRef)->IsActiveForRenderObject(pRenderObject);
    }
  }

  void spVisibilityGroup::AddRenderObject(spRenderObjectCollection* pCollection, spRenderObject* pRenderObject)
  {
    EZ_ASSERT_DEV(pRenderObject != nullptr, "RenderObject cannot be null.");

    if (!pRenderObject->m_VisibilityGroupReference.IsInvalid())
      return;

    pRenderObject->m_VisibilityGroupReference = spRenderNodeReference(pCollection->m_Items.GetCount());
    pCollection->m_Items.PushBack(pRenderObject);

    m_pSceneContext->AddRenderObject(pRenderObject);

    if (pRenderObject->m_pRenderFeature == nullptr)
      m_RenderObjectsWithoutFeatures.PushBack(pRenderObject);
    else
      EvaluateActiveRenderStages(pRenderObject);
  }

  void spVisibilityGroup::RemoveRenderObject(spRenderObjectCollection* pCollection, spRenderObject* pRenderObject)
  {
    EZ_ASSERT_DEV(pRenderObject != nullptr, "RenderObject cannot be null.");

    if (pRenderObject->m_pRenderFeature == nullptr)
      m_RenderObjectsWithoutFeatures.RemoveAndSwap(pRenderObject);

    m_pSceneContext->RemoveRenderObject(pRenderObject);

    const auto& reference = pRenderObject->m_VisibilityGroupReference;
    if (reference.IsInvalid())
      return;

    pCollection->m_Items.RemoveAtAndSwap(reference.GetRef());

    if (reference.GetRef() < pCollection->m_Items.GetCount())
      pCollection->m_Items[reference.GetRef()]->m_VisibilityGroupReference = spRenderNodeReference(reference.GetRef());

    pRenderObject->m_VisibilityGroupReference = spRenderNodeReference::MakeInvalid();
  }

  void spVisibilityGroup::EvaluateActiveRenderStages()
  {
    if (!m_bNeedRenderStageEvaluation)
      return;

    m_bNeedRenderStageEvaluation = false;

    for (auto& pRenderObject : m_RenderObjects.m_Items)
      EvaluateActiveRenderStages(pRenderObject);
  }
} // namespace RPI
