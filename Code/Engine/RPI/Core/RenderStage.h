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

#include <RPI/Core/SortMode.h>

namespace RPI
{
  class spRenderObject;
  class spRenderView;
  class spRenderStage;

  /// \brief Specifies how to filter \a spRenderObject from a \a spRenderStage.
  class SP_RPI_DLL spRenderStageFilter
  {
  public:
    virtual ~spRenderStageFilter() = default;

    /// \brief Returns true if the render object should be visible in the given rendering view.
    /// \param renderObject The render object to check.
    /// \param renderView The rendering view in which check the visibility.
    /// \param renderStage The current rendering stage.
    virtual bool IsVisible(const spRenderObject& renderObject, const spRenderView& renderView, const spRenderStage& renderStage) = 0;
  };

  class SP_RPI_DLL spRenderStage
  {
  public:

  private:
    ezHashedString m_sName;

    ezUniquePtr<spSortMode> m_pSortMode{nullptr};
    ezUniquePtr<spRenderStageFilter> m_pFilter{nullptr};
  };
} // namespace RPI
