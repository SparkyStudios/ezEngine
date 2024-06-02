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

#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderObject.h>
#include <RPI/Core/RenderView.h>
#include <RPI/Core/SortMode.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSortMode, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spDistanceSortMode, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFrontToBackSortMode, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spBackToFrontSortMode, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spSortMode::GenerateSortKeys(const spRenderView* pRenderView, ezArrayPtr<spRenderObject* const> renderObjects, ezArrayPtr<spSortKey> out_sortKeys)
  {
    EZ_ASSERT_DEV(renderObjects.GetCount() == out_sortKeys.GetCount(), "The number of render objects must match the number of sort keys.");

    for (ezUInt32 i = 0, l = renderObjects.GetCount(); i < l; ++i)
      GenerateSortKey(pRenderView, renderObjects[i], out_sortKeys[i]);
  }

  void spDistanceSortMode::GetSortKey(const spRenderObject* pRenderObject, const ezPlane& plane, spSortKey& out_sortKey) const
  {
    const float fDistance = plane.GetDistanceTo(pRenderObject->GetBoundingBox().m_vCenter);
    ezUInt32 uiDistance = ComputeDistance(fDistance);

    if (m_bSortBackToFront)
      uiDistance = ~uiDistance;

    // Compute sort key
    out_sortKey.m_uiValue = uiDistance;
    out_sortKey.m_uiIndex = pRenderObject->GetVisibilityGroupReference().GetRef();
    out_sortKey.m_uiStableIndex = pRenderObject->GetRenderFeatureRefenrence().GetRef();
  }

  void spDistanceSortMode::GenerateSortKey(const spRenderView* pRenderView, const spRenderObject* pRenderObject, spSortKey& out_sortKey)
  {
    const ezMat4 inverseView = pRenderView->GetViewMatrix().GetInverse();
    const ezVec3 vForward = inverseView.GetColumn(2).GetAsVec3();
    const ezPlane plane = ezPlane::MakeFromNormalAndPoint(vForward, inverseView.GetTranslationVector());

    GetSortKey(pRenderObject, plane, out_sortKey);
  }

  void spDistanceSortMode::GenerateSortKeys(const spRenderView* pRenderView, ezArrayPtr<spRenderObject* const> renderObjects, ezArrayPtr<spSortKey> out_sortKeys)
  {
    EZ_ASSERT_DEV(renderObjects.GetCount() == out_sortKeys.GetCount(), "The number of render objects must match the number of sort keys.");

    const ezMat4 inverseView = pRenderView->GetViewMatrix().GetInverse();
    const ezVec3 vForward = inverseView.GetColumn(2).GetAsVec3();
    const ezPlane plane = ezPlane::MakeFromNormalAndPoint(vForward, inverseView.GetTranslationVector());

    for (ezUInt32 i = 0, l = renderObjects.GetCount(); i < l; ++i)
      GetSortKey(renderObjects[i], plane, out_sortKeys[i]);
  }

  spDistanceSortMode::spDistanceSortMode(bool bSortBackToFront)
    : m_bSortBackToFront(bSortBackToFront)
  {
  }

  ezUInt32 spDistanceSortMode::ComputeDistance(float fDistance)
  {
    // Compute uint sort key (http://aras-p.info/blog/2014/01/16/rough-sorting-by-depth/)
    const ezUInt32 uiDistance = *reinterpret_cast<ezUInt32*>(&fDistance);
    return (static_cast<ezUInt32>(-static_cast<ezInt32>(uiDistance >> 31)) | 0x80000000) ^ uiDistance;
  }

  spSortKey spDistanceSortMode::CreateSortKey(float fDistance)
  {
    return spSortKey{ComputeDistance(fDistance)};
  }

  spFrontToBackSortMode::spFrontToBackSortMode()
    : spDistanceSortMode(false)
  {
  }

  spBackToFrontSortMode::spBackToFrontSortMode()
    : spDistanceSortMode(true)
  {
  }
} // namespace RPI
