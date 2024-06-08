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

template <typename T>
constexpr spLazy<T>::spLazy(const T& value)
  : m_bInitialized(true)
  , m_Value(value)
{
}

template <typename T>
constexpr spLazy<T>::spLazy(std::function<T()> initializer)
  : m_Initializer(initializer)
{
}

template <typename T>
const T& spLazy<T>::Get()
{
  if (!m_bInitialized)
  {
    m_Value = m_Initializer();
    m_bInitialized = true;
  }

  return m_Value;
}
