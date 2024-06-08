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

#include <functional>

/// \brief A lazy value that is initialized on first access.
template <typename T>
class spLazy
{
public:
  /// \brief Default constructor.
  constexpr spLazy() = default;

  /// \brief Creates a new initialized lazy value.
  /// \param value The value to initialize with.
  constexpr spLazy(const T& value);

  /// \brief Creates a new uninitialized lazy value.
  /// \param initializer The function to call to initialize the value.
  explicit constexpr spLazy(std::function<T()> initializer);

  /// \brief Gets the value.
  /// If the value has not been initialized, it will be initialized.
  /// \return The value.
  const T& Get();

private:
  std::function<T()> m_Initializer;
  bool m_bInitialized{false};

  T m_Value{T()};
};

#include <Foundation/Types/Implementation/Lazy_inl.h>
