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

#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Features/RenderFeatureExtractor.h>

namespace RPI
{
  struct spRenderSystemCollectEvent;

  class spRenderContext;

  class SP_RPI_DLL spRenderFeature : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spRenderFeature, ezReflectedClass);
    EZ_DISALLOW_COPY_AND_ASSIGN(spRenderFeature);

  public:
    explicit spRenderFeature(ezInternal::NewInstance<spRenderFeatureExtractor> pExtractor);
    ~spRenderFeature() override = default;

    virtual void GetSupportedRenderObjectTypes(ezHybridArray<const ezRTTI*, 8>& out_Types) const = 0;

    virtual void Render(const spRenderContext* pRenderingContext) const = 0;

    /// \brief Called at the initialization of the \a spCompositor to register this feature extractor
    /// in the \a spRenderSystem.
    virtual void RegisterExtractor();

    /// \brief Called at the deinitialization of the \a spCompositor to unregister this feature extractor
    /// from the \a spRenderSystem.
    virtual void UnregisterExtractor();

    /// \brief Gets the name of the feature.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezStringView GetName() const { return m_sName.GetView(); }

    /// \brief Sets the name of the feature.
    /// \param sName The new name to assign to the feature.
    EZ_ALWAYS_INLINE void SetName(ezStringView sName)
    {
      if (!sName.IsEmpty())
        m_sName.Assign(sName);
    }

  protected:
    /// \brief Event handler for the "collect" event of the \a spRenderSystem.
    /// \param event The event data.
    virtual void OnRenderSystemCollectEvent(const spRenderSystemCollectEvent& event);

  private:
    ezHashedString m_sName{};
    bool m_bIsActive{false};

    ezUniquePtr<spRenderFeatureExtractor> m_pExtractor{nullptr};
  };
} // namespace RPI
