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
  class SP_RPI_DLL spSortMode
  {
  public:
    virtual ~spSortMode() = default;

    /// \brief Generates a sort key for a \a spRenderObject.
    /// \param pRenderView The render view.
    /// \param pRenderData The render data.
    /// \param out_sortKey The generated sort key.
    virtual void GenerateSortKey(const spRenderView& pRenderView, const spRenderObject& pRenderData, spSortKey& out_sortKey) = 0;
  };
} // namespace RPI