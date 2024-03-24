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

#include <RPI/Core/RenderView.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_BITFLAGS(RPI::spRenderViewUsage, 1)
  EZ_BITFLAGS_CONSTANT(RPI::spRenderViewUsage::Main),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderViewUsage::ShadowMapping),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderViewUsage::Culling),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderViewUsage::All),
EZ_END_STATIC_REFLECTED_BITFLAGS;
// clang-format on

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderView, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spRenderView::SetData(const spRenderViewData& data)
  {
    m_RenderViewDataBuffer.Set(data);
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Core_RenderView);
