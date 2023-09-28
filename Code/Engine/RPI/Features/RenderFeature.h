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

    /// \brief Called at the initialization of the \a spCompositor.
    /// \param[in] pRenderingContext The rendering context for which the feature is initialized.
    void Initialize(const spRenderContext* pRenderingContext);

    /// \brief Called at the deinitialization of the \a spCompositor.
    void Deinitialize();

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

    /// \brief Called at the initialization of the feature, after the render context is set.
    virtual void OnInitialize();

    /// \brief Called at the deinitialization of the feature, before the render context is cleared.
    virtual void OnDeinitialize();

  private:
    ezHashedString m_sName{};
    bool m_bIsActive{false};
    bool m_bIsInitialized{false};

    ezUniquePtr<spRenderFeatureExtractor> m_pExtractor{nullptr};
    const spRenderContext* m_pRenderContext{nullptr};
  };
} // namespace RPI
