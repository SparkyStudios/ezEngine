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
// limitations under the License.

#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderObject.h>
#include <RPI/Core/VisibilityGroup.h>

namespace RPI
{
  void spRenderObject::Draw(const spRenderContext* pRenderContext)
  {
    if (m_pRenderFeature == nullptr)
      return;

    const ezInt32 iRef = pRenderContext->GetExtractionData().m_pRenderStage->GetRenderSystemReference().GetRef();
    if (m_ActiveRenderStages[iRef] == false)
      return;

    m_pRenderFeature->Draw(this, pRenderContext);
  }

  spRenderObjectCollection::spRenderObjectCollection(spVisibilityGroup* pVisibilityGroup)
    : m_pVisibilityGroup(pVisibilityGroup)
  {
  }

  void spRenderObjectCollection::Add(spRenderObject* pRenderObject)
  {
    m_pVisibilityGroup->AddRenderObject(this, pRenderObject);
  }

  void spRenderObjectCollection::Clear()
  {
    m_Items.Clear();
  }

  bool spRenderObjectCollection::Contains(const spRenderObject* pRenderObject) const
  {
    return m_Items.Contains(const_cast<spRenderObject*>(pRenderObject));
  }

  void spRenderObjectCollection::Remove(spRenderObject* pRenderObject)
  {
    m_pVisibilityGroup->RemoveRenderObject(this, pRenderObject);
  }
} // namespace RPI
