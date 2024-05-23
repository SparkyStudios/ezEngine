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
  struct spSceneContextCollectEvent;

  class spRenderContext;
  class spRenderComponent;

  typedef ezDynamicArray<spRenderObject*> spRenderPacket;

  class SP_RPI_DLL spRenderFeature : public ezReflectedClass
  {
    friend class spRenderSystem;
    friend class spRenderComponent;
    friend class spSceneContext;

    EZ_ADD_DYNAMIC_REFLECTION(spRenderFeature, ezReflectedClass);
    EZ_DISALLOW_COPY_AND_ASSIGN(spRenderFeature);

  public:
    explicit spRenderFeature(ezInternal::NewInstance<spRenderFeatureExtractor> pExtractor);
    ~spRenderFeature() override = default;

    virtual void GetSupportedRenderObjectTypes(ezHybridArray<const ezRTTI*, 8>& out_Types) const = 0;

    virtual void Draw(spRenderObject* pRenderObject, const spRenderContext* pRenderingContext) = 0;

    /// \brief Called at the initialization of the \a spCompositor.
    /// \param [in] pRenderContext The rendering context for which the feature is initialized.
    void Initialize(const spRenderContext* pRenderContext);

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

    /// \brief Extracts a specific set of render objects from the given view.
    /// \param [in] pRenderContext The rendering context in which the extraction occurs.
    void Extract(spSceneContext* pSceneContext, const spRenderContext* pRenderContext) const;

  protected:
    /// \brief Tries to add a \a spRenderObject in the feature.
    /// \note The default implementation always add the render object to the feature
    /// and returns true.
    /// \param [in] pRenderObject The render object to add to the feature.
    /// \return \c true if the render object was added to the feature, \c false otherwise.
    virtual bool TryAddRenderObject(spRenderObject* pRenderObject);

    /// \brief Removes a \a spRenderObject from the feature.
    /// \param [in] pRenderObject The render object to remove from the feature.
    virtual void RemoveRenderObject(spRenderObject* pRenderObject);

    /// \brief Tries to add a \a spRenderComponent in the feature.
    /// \note The default implementation always add the render component to the feature
    /// and returns true.
    /// \param [in] pRenderComponent The render component to add to the feature.
    /// \return \c true if the render component was added to the feature, \c false otherwise.
    virtual bool TryAddRenderComponent(spRenderComponent* pRenderComponent);

    /// \brief Removes a \a spRenderComponent from the feature.
    /// \param [in] pRenderComponent The render component to remove from the feature.
    virtual void RemoveRenderComponent(spRenderComponent* pRenderComponent);

    /// \brief Event handler for the "collect" event of the \a spRenderSystem.
    /// \param [in] event The event data.
    virtual void OnRenderSystemCollectEvent(const spSceneContextCollectEvent& event);

    /// \brief Called at the initialization of the feature, after the render context is set.
    virtual void OnInitialize();

    /// \brief Called at the deinitialization of the feature, before the render context is cleared.
    virtual void OnDeinitialize();

  private:
    ezHashedString m_sName{};
    bool m_bIsActive{true};
    bool m_bIsInitialized{false};

    ezUniquePtr<spRenderFeatureExtractor> m_pExtractor{nullptr};
    const spRenderContext* m_pRenderContext{nullptr};

    ezDynamicArray<spRenderObject*> m_RenderObjects;

    ezDynamicArray<spRenderComponent*> m_RenderComponents;
  };
} // namespace RPI
