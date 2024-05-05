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
  class SP_RPI_DLL spRenderer : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spRenderer, ezReflectedClass);

  public:
    spRenderer(ezStringView sName);
    virtual ~spRenderer() = default;

    virtual void Render() = 0;

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezStringView& GetName() const { return m_sName; }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsEnabled() const { return m_bEnabled; }
    EZ_ALWAYS_INLINE void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsInitialized() const { return m_bInitialized; }

    void Initialize(const spRenderContext* pRenderContext);

  protected:
    EZ_NODISCARD EZ_ALWAYS_INLINE const spRenderContext* GetRenderContext() const { return m_pRenderContext; }

    virtual void OnInitialize();

    bool m_bEnabled{true};

  private:
    ezStringView m_sName;
    bool m_bInitialized{false};
    const spRenderContext* m_pRenderContext{nullptr};
  };
} // namespace RPI
