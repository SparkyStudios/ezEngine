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

#include <RPI/Core/SortKey.h>

namespace RPI
{
  class spRenderObject;
  class spRenderView;

  /// \brief Specifies the sort mode for \a spRenderObject.
  ///
  /// \see spSortKey
  class SP_RPI_DLL spSortMode : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spSortMode, ezReflectedClass);

  public:
    ~spSortMode() override = default;

    /// \brief Generates a sort key for a \a spRenderObject.
    /// \param pRenderView The render view.
    /// \param pRenderObject The render object.
    /// \param out_sortKey The generated sort key.
    virtual void GenerateSortKey(const spRenderView* pRenderView, const spRenderObject* pRenderObject, spSortKey& out_sortKey) = 0;

    /// \brief Generates sort keys for a list of \a spRenderObject.
    /// \param pRenderView The render view.
    /// \param renderObjects The list of render objects.
    /// \param out_sortKeys The generated sort keys.
    virtual void GenerateSortKeys(const spRenderView* pRenderView, ezArrayPtr<spRenderObject* const> renderObjects, ezArrayPtr<spSortKey> out_sortKeys);
  };

  /// \brief Compute a \c spSortKey for a \c spRenderObject based on
  /// the distance between the render object and the render view.
  class SP_RPI_DLL spDistanceSortMode : public spSortMode
  {
    EZ_ADD_DYNAMIC_REFLECTION(spDistanceSortMode, spSortMode);

    // spSortMode

  public:
    ~spDistanceSortMode() override = default;

    void GetSortKey(const spRenderObject* pRenderObject, const ezPlane& plane, spSortKey& out_sortKey) const;
    void GenerateSortKey(const spRenderView* pRenderView, const spRenderObject* pRenderObject, spSortKey& out_sortKey) override;

    void GenerateSortKeys(const spRenderView* pRenderView, ezArrayPtr<spRenderObject* const> renderObjects, ezArrayPtr<spSortKey> out_sortKeys) override;

    // spDistanceSortMode

  public:
    explicit spDistanceSortMode(bool bSortBackToFront = false);

  private:
    static ezUInt32 ComputeDistance(float fDistance);
    static spSortKey CreateSortKey(float fDistance);

    bool m_bSortBackToFront{false};
  };

  class SP_RPI_DLL spFrontToBackSortMode : public spDistanceSortMode
  {
    EZ_ADD_DYNAMIC_REFLECTION(spFrontToBackSortMode, spDistanceSortMode);

    // spFrontToBackSortMode

  public:
    spFrontToBackSortMode();
    ~spFrontToBackSortMode() override = default;
  };

  class SP_RPI_DLL spBackToFrontSortMode : public spDistanceSortMode
  {
    EZ_ADD_DYNAMIC_REFLECTION(spBackToFrontSortMode, spDistanceSortMode);

    // spBackToFrontSortMode

  public:
    spBackToFrontSortMode();
    ~spBackToFrontSortMode() override = default;
  };
} // namespace RPI
