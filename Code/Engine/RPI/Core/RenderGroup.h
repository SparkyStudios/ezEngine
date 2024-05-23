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

namespace RPI
{
  /// \brief A group in which render objects are assigned.
  ///
  /// In a \a spRenderView it's used as a group mask which stores one or more groups, specifying
  /// what are the render objects that should be rendered by this view.
  struct SP_RPI_DLL spRenderGroup
  {
    typedef ezUInt32 StorageType;

    enum Enum : StorageType
    {
      None = 0,

      Group0 = EZ_BIT(0),
      Group1 = EZ_BIT(1),
      Group2 = EZ_BIT(2),
      Group3 = EZ_BIT(3),
      Group4 = EZ_BIT(4),
      Group5 = EZ_BIT(5),
      Group6 = EZ_BIT(6),
      Group7 = EZ_BIT(7),
      Group8 = EZ_BIT(8),
      Group9 = EZ_BIT(9),
      Group10 = EZ_BIT(10),
      Group11 = EZ_BIT(11),
      Group12 = EZ_BIT(12),
      Group13 = EZ_BIT(13),
      Group14 = EZ_BIT(14),
      Group15 = EZ_BIT(15),
      Group16 = EZ_BIT(16),
      Group17 = EZ_BIT(17),
      Group18 = EZ_BIT(18),
      Group19 = EZ_BIT(19),
      Group20 = EZ_BIT(20),
      Group21 = EZ_BIT(21),
      Group22 = EZ_BIT(22),
      Group23 = EZ_BIT(23),
      Group24 = EZ_BIT(24),
      Group25 = EZ_BIT(25),
      Group26 = EZ_BIT(26),
      Group27 = EZ_BIT(27),
      Group28 = EZ_BIT(28),
      Group29 = EZ_BIT(29),
      Group30 = EZ_BIT(30),
      Group31 = static_cast<ezUInt32>(EZ_BIT(31)),

      All = UINT32_MAX,

      Default = All,
    };

    struct Bits
    {
      StorageType Group0 : 1;
      StorageType Group1 : 1;
      StorageType Group2 : 1;
      StorageType Group3 : 1;
      StorageType Group4 : 1;
      StorageType Group5 : 1;
      StorageType Group6 : 1;
      StorageType Group7 : 1;
      StorageType Group8 : 1;
      StorageType Group9 : 1;
      StorageType Group10 : 1;
      StorageType Group11 : 1;
      StorageType Group12 : 1;
      StorageType Group13 : 1;
      StorageType Group14 : 1;
      StorageType Group15 : 1;
      StorageType Group16 : 1;
      StorageType Group17 : 1;
      StorageType Group18 : 1;
      StorageType Group19 : 1;
      StorageType Group20 : 1;
      StorageType Group21 : 1;
      StorageType Group22 : 1;
      StorageType Group23 : 1;
      StorageType Group24 : 1;
      StorageType Group25 : 1;
      StorageType Group26 : 1;
      StorageType Group27 : 1;
      StorageType Group28 : 1;
      StorageType Group29 : 1;
      StorageType Group30 : 1;
      StorageType Group31 : 1;
    };
  };
} // namespace RPI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spRenderGroup);
