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

#pragma once

/// \brief Enables a feature checked with \a SP_ENABLED
#define SP_ON 1

/// \brief Disables a feature checked with \a SP_ENABLED
#define SP_OFF 0

/// \brief Checks if the specified feature is enabled
#define SP_ENABLED(_MACRO_) _MACRO_ == SP_ON

/// \brief Checks if the specified feature is disabled
#define SP_DISABLED(_MACRO_) _MACRO_ == SP_OFF
