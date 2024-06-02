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

#include <RPI/Core/RenderStage.h>

namespace RPI
{
  struct spSortKeyComprarer
  {
    typedef const spRenderObject* T;

    spSortKeyComprarer(const spRenderView* pRenderView, spSortMode* pSortMode)
    {
      m_pRenderView = pRenderView;
      m_pSortMode = pSortMode;
    }

    /// \brief Returns true if a is less than b
    EZ_ALWAYS_INLINE bool Less(const T& a, const T& b) const
    {
      spRenderObject* objects[] = {const_cast<spRenderObject*>(a), const_cast<spRenderObject*>(b)};
      spSortKey keys[2] = {};

      m_pSortMode->GenerateSortKeys(m_pRenderView, ezMakeArrayPtr(objects), ezMakeArrayPtr(keys));

      return keys[0] < keys[1];
    }

    /// \brief Returns true if a is equal to b
    EZ_ALWAYS_INLINE bool Equal(const T& a, const T& b) const
    {
      spRenderObject* objects[] = {const_cast<spRenderObject*>(a), const_cast<spRenderObject*>(b)};
      spSortKey keys[2] = {};

      m_pSortMode->GenerateSortKeys(m_pRenderView, ezMakeArrayPtr(objects), ezMakeArrayPtr(keys));

      return keys[0] == keys[1];
    }

    const spRenderView* m_pRenderView;
    spSortMode* m_pSortMode;
  };

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderStageFilter, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderStage, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spRenderStage::spRenderStage(ezStringView sName)
  {
    m_sName.Assign(sName);
    spRenderSystem::GetSingleton()->RegisterRenderStage(this);
  }

  void spRenderStage::Filter(const spRenderView* pRenderView, const ezArrayPtr<spRenderObject* const>& renderObjects, ezDynamicArray<spRenderObject*>* out_passedRenderObjects, ezDynamicArray<spRenderObject*>* out_failedRenderObjects)
  {
    if (out_passedRenderObjects == nullptr && out_failedRenderObjects == nullptr)
      return;

    if (m_pFilter == nullptr && out_passedRenderObjects != nullptr)
    {
      *out_passedRenderObjects = renderObjects;
      return;
    }

    for (const auto& pRenderObject : renderObjects)
    {
      if (out_passedRenderObjects != nullptr && m_pFilter->IsVisible(pRenderObject, pRenderView, this))
        out_passedRenderObjects->PushBack(pRenderObject);
      else if (out_failedRenderObjects != nullptr)
        out_failedRenderObjects->PushBack(pRenderObject);
    }
  }

  void spRenderStage::Sort(const spRenderView* pRenderView, const ezArrayPtr<spRenderObject* const>& renderObjects, ezDynamicArray<spRenderObject*>& out_sortedRenderObjects)
  {
    out_sortedRenderObjects = renderObjects;

    const spSortKeyComprarer comparer(pRenderView, m_pSortMode.Borrow());
    out_sortedRenderObjects.Sort(comparer);
  }

  RHI::spOutputDescription spRenderStage::GetOutputDescription(const spRenderView* pRenderView) const
  {
    const ezUInt32 uiIndex = m_RenderViewFramebuffers.Find(pRenderView);
    if (uiIndex == ezInvalidIndex)
      return {};

    return m_RenderViewFramebuffers.GetValue(uiIndex)->GetOutputDescription();
  }

  ezSharedPtr<RHI::spFramebuffer> spRenderStage::GetOutputFramebuffer(const spRenderView* pRenderView) const
  {
    const ezUInt32 uiIndex = m_RenderViewFramebuffers.Find(pRenderView);
    if (uiIndex == ezInvalidIndex)
      return nullptr;

    return m_RenderViewFramebuffers.GetValue(uiIndex);
  }
} // namespace RPI
