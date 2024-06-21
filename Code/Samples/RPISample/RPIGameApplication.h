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

#include <GameEngine/GameApplication/GameApplication.h>

#include <RAI/Resources/ShaderResource.h>

#include <RPI/Composition/Compositor.h>
#include <RPI/Core/RenderSystem.h>
#include <RPI/Core/Threading/RenderThread.h>
#include <RPI/Graph/RenderGraph.h>
#include <RPI/Scene/SceneContext.h>

using namespace RHI;
using namespace RPI;

class spRPISampleWindow;

class spRPIGameApplication : public ezGameApplication
{
  using SUPER = ezGameApplication;

public:
  spRPIGameApplication();

  void AfterCoreSystemsStartup() override;
  void BeforeHighLevelSystemsShutdown() override;

  void OnResize(ezUInt32 width, ezUInt32 height);

protected:
  void Init_LoadRequiredPlugins() override;
  void Init_ConfigureInput() override;
  void Init_SetupDefaultResources() override;
  void Init_SetupGraphicsDevice() override;
  void Deinit_ShutdownGraphicsDevice() override;


  ezApplication::Execution Run() override;
  bool IsGameUpdateEnabled() const override;

  bool Run_ProcessApplicationInput() override;
  void Run_WorldUpdateAndRender() override;
  void Run_PresentImage() override;
  void Run_FinishFrame() override;

private:
  ezDynamicArray<ezGameObject*> m_Objects;
  ezGameObject* m_pCamera{nullptr};

  ezUniquePtr<ezWorld> m_pWorld{nullptr};
  ezUniquePtr<spSceneContext> m_pSceneContext{nullptr};
  ezUniquePtr<spRenderer> m_pRenderer;

  ezUniquePtr<spRPISampleWindow> m_pWindow{nullptr};
};


class spDemoRenderGraphNode final : public spRenderGraphNode
{
public:
  spDemoRenderGraphNode()
    : spRenderGraphNode("DemoRenderGraphNode")
  {
  }

  ezResult Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources) override;

  ezUniquePtr<spRenderPass> Compile(spRenderGraphBuilder* pBuilder) override;

  bool IsEnabled() const override;

  [[nodiscard]] EZ_ALWAYS_INLINE spResourceHandle GetTarget() const { return m_hRenderTarget; }

private:
  spResourceHandle m_hRenderTarget;
  spResourceHandle m_hShader;

  RAI::spShaderResourceHandle m_hShaderAsset;
};
