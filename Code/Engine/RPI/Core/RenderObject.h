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

#include <RPI/Core/RenderGroup.h>
#include <RPI/Core/RenderNodeReference.h>

#include <Foundation/Math/BoundingBoxSphere.h>

namespace RPI
{
  class spRenderContext;
  class spRenderFeature;
  class spVisibilityGroup;

  /// \brief Base class for render objects. Each implementation must contains all information
  /// needed to render the data. The \a Draw method is called each time this object is rendered.
  class SP_RPI_DLL spRenderObject : public ezReflectedClass
  {
    friend class spRenderSystem;
    friend class spRenderFeature;
    friend class spRenderObjectCollection;
    friend class spVisibilityGroup;
    friend class spSceneContext;
    friend class spRenderComponent;

    EZ_ADD_DYNAMIC_REFLECTION(spRenderObject, ezReflectedClass);

  public:
    /// \brief Specifies the caching behavior used by the render object.
    struct CachingBehavior
    {
      typedef ezUInt8 StorageType;

      enum Enum : StorageType
      {
        /// \brief The data is never cached. It will be computed each time on each frames.
        Never,

        /// \brief The data is cached only if the owner game object is static. This may improve
        /// performances.
        OnlyIfStatic,

        /// \brief The data is always cached. Implementations must provide a custom way to
        /// invalidate the cache.
        Always
      };
    };

    spRenderObject() = default;
    ~spRenderObject() override = default;

    virtual void Draw(const spRenderContext* pRenderContext);

    virtual bool CanBeInstanceOf(spRenderObject* pRenderObject) const;

    /// \brief Transforms the given render object into an instance of this one.
    /// \param pRenderObject The render object to transform.
    /// \returns \c EZ_SUCCESS on success, \c EZ_FAILURE otherwise.
    virtual ezResult Instantiate(spRenderObject* pRenderObject);

    virtual void MakeRootInstance();

    virtual bool HasInstances();

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezBoundingBoxSphere& GetBoundingBox() const { return m_BoundingBox; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const spRenderNodeReference& GetVisibilityGroupReference() const { return m_VisibilityGroupReference; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spRenderNodeReference& GetRenderFeatureRefenrence() const { return m_RenderFeatureReference; }

  protected:
    ezEnum<CachingBehavior> m_eCachingBehavior{CachingBehavior::OnlyIfStatic};
    ezEnum<spRenderGroup> m_eRenderGroup{spRenderGroup::None};
    ezBoundingBoxSphere m_BoundingBox;
    ezUInt64 m_uiUniqueID{0};

    spRenderFeature* m_pRenderFeature{nullptr};

    spRenderNodeReference m_VisibilityGroupReference{spRenderNodeReference::MakeInvalid()};
    spRenderNodeReference m_RenderFeatureReference{spRenderNodeReference::MakeInvalid()};

    ezHybridArray<bool, 16> m_ActiveRenderStages;
  };

  /// \brief A collection of \a spRenderObject.
  class SP_RPI_DLL spRenderObjectCollection
  {
    friend class spVisibilityGroup;

  public:
    explicit spRenderObjectCollection(spVisibilityGroup* pVisibilityGroup);

    void Add(spRenderObject* pRenderObject);

    void Clear();

    EZ_NODISCARD bool Contains(const spRenderObject* pRenderObject) const;

    void Remove(spRenderObject* pRenderObject);

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetCount() const { return m_Items.GetCount(); }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsEmpty() const { return m_Items.IsEmpty(); }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezArrayPtr<spRenderObject* const> GetItems() const { return m_Items.GetArrayPtr(); }

  private:
    ezHybridArray<spRenderObject*, 64> m_Items;
    spVisibilityGroup* m_pVisibilityGroup{nullptr};
  };
} // namespace RPI
