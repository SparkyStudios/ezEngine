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
  spVisibilityGroup::spVisibilityGroup(spRenderSystem* pRenderSystem)
    : m_RenderObjectsWithoutFeatures()
    , m_pRenderSystem(pRenderSystem)
  {
  }

  spVisibilityGroup::~spVisibilityGroup()
  {
  }

  void spVisibilityGroup::Reset()
  {
    // TODO
  }

  void spVisibilityGroup::Collect(spRenderView* pView)
  {
    // TODO
  }

  void spVisibilityGroup::Copy(spRenderView* pSource, spRenderView* pDestination)
  {
    // TODO
  }

  void spVisibilityGroup::AddRenderObject(spRenderObjectCollection* pCollection, spRenderObject* pRenderObject)
  {
    EZ_ASSERT_DEV(pRenderObject != nullptr, "RenderObject cannot be null.");

    if (!pRenderObject->m_VisibilityGroupReference.IsInvalid())
      return;

    pRenderObject->m_VisibilityGroupReference = spRenderNodeReference(pCollection->m_Items.GetCount());
    pCollection->m_Items.PushBack(pRenderObject);

    m_pRenderSystem->AddRenderObject(pRenderObject);

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

    m_pRenderSystem->RemoveRenderObject(pRenderObject);
  }

  void spVisibilityGroup::EvaluateActiveRenderStages(spRenderObject* pRenderObject)
  {
    // TODO
  }
} // namespace RPI
