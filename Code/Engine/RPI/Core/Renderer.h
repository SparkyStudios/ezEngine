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
    explicit spRenderer(ezStringView sName);
    ~spRenderer() override = default;

    virtual void Render() = 0;
    virtual void Prepare() = 0;

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezStringView& GetName() const { return m_sName; }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsEnabled() const { return m_bEnabled; }
    EZ_ALWAYS_INLINE void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsInitialized() const { return m_bInitialized; }

    virtual void Initialize(const spSceneContext* pSceneContext);

  protected:
    EZ_NODISCARD EZ_ALWAYS_INLINE const spSceneContext* GetSceneContext() const { return m_pSceneContext; }

    virtual void OnInitialize();

    bool m_bEnabled{true};

  private:
    ezStringView m_sName;
    bool m_bInitialized{false};
    const spSceneContext* m_pSceneContext{nullptr};
  };

  class SP_RPI_DLL spParentRenderer : public spRenderer
  {
    EZ_ADD_DYNAMIC_REFLECTION(spParentRenderer, spRenderer);

    // spRenderer

  public:
    void Render() override;
    void Prepare() override;
    void Initialize(const spSceneContext* pSceneContext) override;

    // spSceneRenderer

  public:
    explicit spParentRenderer(ezStringView sName);
    ~spParentRenderer() override = default;

#pragma region Properties

    EZ_ALWAYS_INLINE void SetChildRenderer(spRenderer* pChildRenderer) { m_pChildRenderer = pChildRenderer; }
    EZ_NODISCARD EZ_ALWAYS_INLINE spRenderer* GetChildRenderer() const { return m_pChildRenderer; }

#pragma endregion

  private:
    spRenderer* m_pChildRenderer{nullptr};
  };
} // namespace RPI
