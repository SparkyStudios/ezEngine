// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <RPI/Core/RenderGroup.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(RPI::spRenderGroup, 1)
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group0),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group1),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group2),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group3),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group4),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group5),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group6),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group7),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group8),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group9),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group10),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group11),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group12),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group13),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group14),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group15),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group16),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group17),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group18),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group19),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group20),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group21),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group22),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group23),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group24),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group25),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group26),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group27),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group28),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group29),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group30),
  EZ_ENUM_CONSTANT(RPI::spRenderGroup::Group31),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_BITFLAGS(RPI::spRenderGroupMask, 1)
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group1),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group0),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group2),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group3),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group4),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group5),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group6),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group7),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group8),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group9),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group10),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group11),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group12),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group13),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group14),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group15),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group16),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group17),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group18),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group19),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group20),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group21),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group22),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group23),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group24),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group25),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group26),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group27),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group28),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group29),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group30),
  EZ_BITFLAGS_CONSTANT(RPI::spRenderGroupMask::Group31),
EZ_END_STATIC_REFLECTED_BITFLAGS;
// clang-format on

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Core_RenderGroup);
