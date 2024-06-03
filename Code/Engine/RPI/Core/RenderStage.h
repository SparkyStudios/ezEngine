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

#include <RPI/Core/RenderNodeReference.h>
#include <RPI/Core/SortMode.h>

#include <RHI/Framebuffer.h>
#include <RHI/Output.h>
#include <RHI/Rendering.h>

namespace RPI
{
  class spRenderObject;
  class spRenderView;
  class spRenderStage;
  class spRenderContext;

  /// \brief Specifies how to filter \a spRenderObject from a \a spRenderStage.
  class SP_RPI_DLL spRenderStageFilter : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spRenderStageFilter, ezReflectedClass);

  public:
    ~spRenderStageFilter() override = default;

    /// \brief Returns true if the render object should be visible in the given rendering view.
    /// \param pRenderObject The render object to check.
    /// \param pRenderView The rendering view in which check the visibility.
    /// \param pRenderStage The current rendering stage.
    virtual bool IsVisible(const spRenderObject* pRenderObject, const spRenderView* pRenderView, const spRenderStage* pRenderStage) = 0;
  };

  /// \brief A single stage in a rendering feature.
  ///
  /// A stage specifies how to render a set of render objects from a single rendering view.
  /// For example, the \a spMeshRenderFeature renders objects using the GBuffer stage to
  /// render opaque objects, and an Opacity stage to render the transparent objects.
  ///
  /// The render stage also filter the render objects eligible for the stage from a given
  /// rendering view and sort them using a \a spSortMode.
  ///
  /// \see spRenderFeature
  /// \see spRenderView
  /// \see spRenderObject
  /// \see spSortMode
  /// \see spRenderStageFilter
  class SP_RPI_DLL spRenderStage : public ezReflectedClass
  {
    friend class spRenderSystem;

    EZ_ADD_DYNAMIC_REFLECTION(spRenderStage, ezReflectedClass);

  public:
    explicit spRenderStage(ezStringView sName);
    ~spRenderStage() override = default;

    virtual void Filter(const spRenderView* pRenderView, const ezArrayPtr<spRenderObject* const>& renderObjects, ezDynamicArray<spRenderObject*>* out_passedRenderObjects, ezDynamicArray<spRenderObject*>* out_failedRenderObjects);
    virtual void Sort(const spRenderView* pRenderView, const ezArrayPtr<spRenderObject* const>& renderObjects, ezDynamicArray<spRenderObject*>& out_sortedRenderObjects);

    virtual void Draw(const spRenderContext* pRenderContext, const ezArrayPtr<spRenderObject* const>& renderObjects);

    virtual bool IsActiveForRenderObject(const spRenderObject* pRenderObject) const = 0;

    virtual RHI::spOutputDescription GetOutputDescription(const spRenderView* pRenderView) const;
    virtual RHI::spRenderingState GetRenderingState(const spRenderObject* pRenderObject) const = 0;

    EZ_NODISCARD ezSharedPtr<RHI::spFramebuffer> GetOutputFramebuffer(const spRenderView* pRenderView) const;

    virtual void CreateOutputFramebuffer(const spRenderView* pRenderView) = 0;

    EZ_NODISCARD EZ_ALWAYS_INLINE spRenderNodeReference GetRenderSystemReference() const { return m_RenderSystemReference; }

  protected:
    virtual void BatchInstances(const ezArrayPtr<spRenderObject* const>& renderObjects, ezDynamicArray<spRenderObject*>& out_batchedRenderObjects);

    ezHashedString m_sName;

    ezUniquePtr<spSortMode> m_pSortMode{nullptr};
    ezUniquePtr<spRenderStageFilter> m_pFilter{nullptr};
    ezArrayMap<const spRenderView*, ezSharedPtr<RHI::spFramebuffer>> m_RenderViewFramebuffers;

    spRenderNodeReference m_RenderSystemReference{spRenderNodeReference::MakeInvalid()};
  };
} // namespace RPI
