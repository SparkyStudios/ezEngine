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

#include <RPI/Core/RenderObject.h>

namespace RPI
{
  class spRenderContext;
  class spRenderView;

  /// \brief Extracts needed data for processing with a \a spRenderFeature.
  class SP_RPI_DLL spRenderFeatureExtractor : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spRenderFeatureExtractor, ezReflectedClass);

  public:
    spRenderFeatureExtractor() = default;
    ~spRenderFeatureExtractor() override = default;

    /// \brief Extracts a specific set of render objects from the given view.
    /// \param [in] pRenderContext The rendering context in which the extraction occurs.
    /// \param [in] pRenderView The render view from which extract render objects.
    virtual void Extract(const spRenderContext* pRenderContext, const spRenderView* pRenderView);
  };
} // namespace RPI
