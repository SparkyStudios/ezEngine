#include "RPIGameApplication.h"

#include <Core/Input/InputManager.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Core/System/ControllerInput.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Time/Clock.h>

#include <RHI/Core.h>
#include <RHI/Factory.h>

#include <RPI/Camera/CameraComponent.h>
#include <RPI/Graph/Nodes/MainSwapchainRenderGraphNode.h>
#include <RPI/Meshes/MeshComponent.h>
#include <RPI/Pipeline/RenderPass.h>
#include <RPI/Renderers/CameraRenderer.h>
#include <RPI/Renderers/ClearRenderer.h>
#include <RPI/Renderers/DeferredRenderer.h>
#include <RPI/Renderers/SceneRenderer.h>
#include <RPI/Renderers/SequenceRenderer.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)
#  include <RHID3D11/Device.h>
#  include <RHID3D11/ResourceManager.h>
#  include <RHID3D11/Swapchain.h>
#elif EZ_ENABLED(EZ_PLATFORM_OSX)
#  include <RHIMTL/Device.h>
#  include <RHIMTL/Swapchain.h>
#endif

static ezUInt32 g_uiWindowWidth = 640;
static ezUInt32 g_uiWindowHeight = 480;

ezResult spDemoRenderGraphNode::Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources)
{
  m_hShaderAsset = ezResourceManager::LoadResource<RAI::spShaderResource>(":project/Shaders/sample.slang");

  const ezResourceLock resource(m_hShaderAsset, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (!resource.IsValid())
  {
    ezLog::Error("Unable to get the image resource! Make sure to run AssetProcessor first.");
    return EZ_FAILURE;
  }

  spRenderTargetDescription rtDescription{};
  rtDescription.m_bGenerateMipMaps = false;
  rtDescription.m_eQuality = spRenderTargetQuality::LDR;
  rtDescription.m_eSampleCount = spTextureSampleCount::None;
  rtDescription.m_uiHeight = g_uiWindowHeight;
  rtDescription.m_uiWidth = g_uiWindowWidth;

  m_hRenderTarget = pBuilder->CreateRenderTarget(this, rtDescription, spRenderGraphResourceBindType::ReadOnly);

  return EZ_SUCCESS;
}

ezUniquePtr<spRenderPass> spDemoRenderGraphNode::Compile(spRenderGraphBuilder* pBuilder)
{
  spRenderGraphResource* rtt = nullptr;
  pBuilder->GetResources().TryGetValue(m_hRenderTarget.GetInternalID(), rtt);
  rtt->m_pRHIResource->SetDebugName("main_render_target");

  auto target = m_hRenderTarget;
  auto shader = m_hShaderAsset;
  const spCallbackRenderPass::ExecuteCallback executeCallback = [target, shader](const spRenderGraphResourcesTable& resources, const spRenderContext* context, ezVariant& passData) -> void
  {
    spRenderGraphResource* rtt = nullptr;
    resources.TryGetValue(target.GetInternalID(), rtt);

    const auto cl = context->GetCommandList();

    const auto c = ezAngle::MakeFromRadian(ezTime::Now().AsFloatInSeconds());
    const auto col = ezColor(ezMath::Sin(c), ezMath::Cos(c), ezMath::Sin(-c), 1.0f);

    spShaderCompilerSetup setup;
    setup.m_eStage = spShaderStage::VertexShader;
    setup.m_SpecializationConstants.PushBack(spShaderSpecializationConstant(ezMakeHashedString("TestSpec"), true));
    setup.m_PredefinedMacros.PushBack({"USE_NORMAL", "1"});

    Slang::ComPtr<slang::IComponentType> shaderProgram;
    context->GetShaderManager()->CompileShader(shader, setup, shaderProgram.writeRef());

    ezSharedPtr<spScopeProfiler> pTestScopeProfiler;

    cl->PushProfileScope("DemoNode");
    {
      cl->SetFramebuffer(rtt->m_pRHIResource.Downcast<spRenderTarget>()->GetFramebuffer());

      cl->SetFullViewport(0);
      cl->SetFullScissorRect(0);

      cl->ClearColorTarget(0, col);
    }
    cl->PopProfileScope(pTestScopeProfiler);
  };

  const spCallbackRenderPass::CleanUpCallback cleanUpCallback = [target](const spRenderGraphResourcesTable& resources, ezVariant& passData) -> void
  {
    spRenderGraphResource* rtt = nullptr;
    resources.TryGetValue(target.GetInternalID(), rtt);
    rtt->m_pRHIResource.Clear();
  };

  ezUniquePtr<spRenderPass> pPass = EZ_NEW(pBuilder->GetAllocator(), spCallbackRenderPass, executeCallback, cleanUpCallback);

  return pPass;
}

bool spDemoRenderGraphNode::IsEnabled() const
{
  return true;
}

class spRPISampleWindow : public ezWindow
{
public:
  spRPISampleWindow(spRPIGameApplication* pApp)
    : ezWindow()
  {
    m_pApp = pApp;
    m_bCloseRequested = false;
  }

  void OnClickClose() override { m_bCloseRequested = true; }

  void OnResize(const ezSizeU32& newWindowSize) override
  {
    if (m_pApp)
    {
      m_CreationDescription.m_Resolution = newWindowSize;
      m_pApp->OnResize(m_CreationDescription.m_Resolution.width, m_CreationDescription.m_Resolution.height);
    }
  }

  bool m_bCloseRequested;

private:
  spRPIGameApplication* m_pApp = nullptr;
};

spRPIGameApplication::spRPIGameApplication()
  : ezGameApplication("RPI Sample", "Data/Samples/RHISample")
{
}

void spRPIGameApplication::AfterCoreSystemsStartup()
{
  ezLog::Debug(ezFileSystem::GetSdkRootDirectory());

  ExecuteInitFunctions();

  ezWorldDesc desc("RPISampleWorld");
  desc.m_uiRandomNumberGeneratorSeed = 42;

  m_pWorld = EZ_DEFAULT_NEW(ezWorld, desc);
  m_pWorld->GetClock().SetFixedTimeStep(ezTime::MakeFromSeconds(1.0 / 60.0));

  m_pSceneContext = ezUniquePtr<spSceneContext>(
    spRenderSystem::GetSingleton()->CreateSceneForWorld(m_pWorld.Borrow()),
    spDeviceAllocatorWrapper::GetAllocator());

  spRenderSystem::GetSingleton()->GetCompositor()->m_pGameRenderer->Initialize(m_pSceneContext.Borrow());

  //  ActivateGameState(m_pWorld.Borrow()).IgnoreResult();

  ezUniquePtr<spRenderGraphBuilder> builder = EZ_NEW(spDeviceAllocatorWrapper::GetAllocator(), spRenderGraphBuilder, spRenderSystem::GetSingleton()->GetDevice());

  {
    ezUniquePtr<spDemoRenderGraphNode> triangleNode = EZ_NEW(spDeviceAllocatorWrapper::GetAllocator(), spDemoRenderGraphNode);
    builder->AddNode("DemoNode", std::move(triangleNode));
  }

  {
    ezUniquePtr<spMainSwapchainRenderGraphNode> swapchainNode = EZ_NEW(spDeviceAllocatorWrapper::GetAllocator(), spMainSwapchainRenderGraphNode);
    swapchainNode->SetRenderTargetSize(g_uiWindowWidth, g_uiWindowHeight);

    ezHashTable<ezHashedString, spResourceHandle> swapchainNodeResources;
    swapchainNodeResources.Insert(ezMakeHashedString("Input"), static_cast<const spDemoRenderGraphNode*>(builder->GetNode("DemoNode"))->GetTarget());

    builder->AddNode("Swapchain", std::move(swapchainNode), swapchainNodeResources);
  }

  m_pSceneContext->AddPipeline(builder->Compile());

  {
    EZ_LOCK(m_pWorld->GetWriteMarker());

    m_Objects.SetCount(3);

    {
      ezGameObjectDesc cubeDesc;
      cubeDesc.m_sName.Assign("Cube1");
      cubeDesc.m_LocalPosition = ezVec3(+50.0f, 0.0f, 0.0f);
      cubeDesc.m_LocalUniformScaling = 1.0f;
      m_pWorld->CreateObject(cubeDesc, m_Objects[0]);

      ezComponentHandle hComponent = m_pWorld->GetOrCreateComponentManager<spMeshComponentManager>()->CreateComponent(m_Objects[0]);
      if (spMeshComponent* pComponent = nullptr; m_pWorld->GetOrCreateComponentManager<spMeshComponentManager>()->TryGetComponent(hComponent, pComponent))
        pComponent->SetMeshFile(":project/objects/teapot.spMesh");
    }

    {
      ezGameObjectDesc cubeDesc;
      cubeDesc.m_sName.Assign("Cube1");
      cubeDesc.m_LocalPosition = ezVec3(+50.0f, -300.0f, 0.0f);
      cubeDesc.m_LocalUniformScaling = 1.0f;
      m_pWorld->CreateObject(cubeDesc, m_Objects[1]);

      ezComponentHandle hComponent = m_pWorld->GetOrCreateComponentManager<spMeshComponentManager>()->CreateComponent(m_Objects[1]);
      if (spMeshComponent* pComponent = nullptr; m_pWorld->GetOrCreateComponentManager<spMeshComponentManager>()->TryGetComponent(hComponent, pComponent))
        pComponent->SetMeshFile(":project/objects/teapot.spMesh");
    }

    {
      ezGameObjectDesc cubeDesc;
      cubeDesc.m_sName.Assign("Cube2");
      cubeDesc.m_LocalPosition = ezVec3(-50.0f, 0.0f, 0.0f);
      cubeDesc.m_LocalUniformScaling = 50.0f;
      m_pWorld->CreateObject(cubeDesc, m_Objects[2]);

      ezComponentHandle hComponent = m_pWorld->GetOrCreateComponentManager<spMeshComponentManager>()->CreateComponent(m_Objects[2]);
      if (spMeshComponent* pComponent = nullptr; m_pWorld->GetOrCreateComponentManager<spMeshComponentManager>()->TryGetComponent(hComponent, pComponent))
        pComponent->SetMeshFile(":project/objects/male.spMesh");
    }

    {
      ezGameObjectDesc cameraDesc;
      cameraDesc.m_sName.Assign("Camera");
      cameraDesc.m_bDynamic = true;
      cameraDesc.m_LocalPosition = ezVec3(-550.0f, 0.0f, 0.0f);
      m_pWorld->CreateObject(cameraDesc, m_pCamera);

      ezComponentHandle hComponent = m_pWorld->GetOrCreateComponentManager<spCameraComponentManager>()->CreateComponent(m_pCamera);
      if (spCameraComponent* pComponent = nullptr; m_pWorld->GetOrCreateComponentManager<spCameraComponentManager>()->TryGetComponent(hComponent, pComponent))
      {
        pComponent->SetCameraSlot("Main");
        pComponent->SetCullingEnabled(true);
      }
    }
  }
}

void spRPIGameApplication::BeforeHighLevelSystemsShutdown()
{
  spRenderSystem::GetSingleton()->UnregisterSceneForWorld(m_pWorld.Borrow());
  m_pWorld = nullptr;

  ezGameApplicationBase::BeforeHighLevelSystemsShutdown();

  m_pSceneContext->CleanUp();
}

void spRPIGameApplication::OnResize(ezUInt32 width, ezUInt32 height)
{
  spRenderSystem::GetSingleton()->GetDevice()->ResizeSwapchain(width, height);
}

void spRPIGameApplication::Init_LoadRequiredPlugins()
{
  ezPlugin::InitializeStaticallyLinkedPlugins();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  ezPlugin::LoadPlugin("ezInspectorPlugin").IgnoreResult();
#endif
}

void spRPIGameApplication::Init_ConfigureInput()
{
  ezGameApplication::Init_ConfigureInput();

  ezInputActionConfig config;

  config.m_sInputSlotTrigger[0] = ezInputSlot_KeyDown;
  ezInputManager::SetInputActionConfig("Main", "MoveCamBack", config, true);

  config.m_sInputSlotTrigger[0] = ezInputSlot_KeyUp;
  ezInputManager::SetInputActionConfig("Main", "MoveCamForward", config, true);

  config.m_sInputSlotTrigger[0] = ezInputSlot_KeyLeft;
  ezInputManager::SetInputActionConfig("Main", "MoveCamLeft", config, true);

  config.m_sInputSlotTrigger[0] = ezInputSlot_KeyRight;
  ezInputManager::SetInputActionConfig("Main", "MoveCamRight", config, true);
}

void spRPIGameApplication::Init_SetupDefaultResources()
{
  // noop
}

void spRPIGameApplication::Init_SetupGraphicsDevice()
{
  ezEnum<spGraphicsApi> eApiType;

  ezStringView szRendererName = ezCommandLineUtils::GetGlobalInstance()->GetStringOption("-rhi", 0, "MTL");
  {
    if (szRendererName.Compare("D3D11") == 0)
    {
      eApiType = spGraphicsApi::Direct3D11;
    }

    else if (szRendererName.Compare("D3D12") == 0)
    {
      eApiType = spGraphicsApi::Direct3D12;
    }

    else if (szRendererName.Compare("VK") == 0)
    {
      eApiType = spGraphicsApi::Vulkan;
    }

    else if (szRendererName.Compare("GL") == 0)
    {
      eApiType = spGraphicsApi::OpenGL;
    }

    else if (szRendererName.Compare("GLES") == 0)
    {
      eApiType = spGraphicsApi::OpenGLES;
    }

    else if (szRendererName.Compare("MTL") == 0)
    {
      eApiType = spGraphicsApi::Metal;
    }
  }

  // Create a window for rendering
  {
    ezWindowCreationDesc WindowCreationDesc;
    WindowCreationDesc.m_Resolution.width = g_uiWindowWidth;
    WindowCreationDesc.m_Resolution.height = g_uiWindowHeight;
    WindowCreationDesc.m_Title = ezStringBuilder("RPISample ", szRendererName);
    WindowCreationDesc.m_bShowMouseCursor = true;
    WindowCreationDesc.m_bClipMouseCursor = false;
    WindowCreationDesc.m_WindowMode = ezWindowMode::WindowResizable;
    m_pWindow = EZ_DEFAULT_NEW(spRPISampleWindow, this);
    m_pWindow->Initialize(WindowCreationDesc).IgnoreResult();
  }

  // Init the rendering system
  {
    spRenderSystem* pRenderSystem = EZ_DEFAULT_NEW(RPI::spRenderSystem);

    if (eApiType == spGraphicsApi::Direct3D11)
    {
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)
      spRenderingSurfaceWin32 renderSurface(ezMinWindows::ToNative(m_pWindow->GetNativeWindowHandle()), nullptr, m_pWindow->IsFullscreenWindow());

      spDeviceDescriptionD3D11 description{};
      description.m_bDebug = true;
      description.m_bSyncV = false;
      description.m_bUsSrgbFormat = true;
      description.m_uiWidth = g_uiWindowWidth;
      description.m_uiHeight = g_uiWindowHeight;
      description.m_bHasMainSwapchain = true;
      description.m_MainSwapchainDescription.m_uiWidth = g_uiWindowWidth;
      description.m_MainSwapchainDescription.m_uiHeight = g_uiWindowHeight;
      description.m_MainSwapchainDescription.m_bUseSrgb = false;
      description.m_MainSwapchainDescription.m_bVSync = false;
      description.m_MainSwapchainDescription.m_bUseDepthTexture = true;
      description.m_MainSwapchainDescription.m_pRenderingSurface = &renderSurface;
      description.m_MainSwapchainDescription.m_eDepthFormat = spPixelFormat::D24UNormS8UInt;
      description.m_bMainSwapchainHasDepth = true;
      description.m_eSwapchainDepthPixelFormat = spPixelFormat::D24UNormS8UInt;
      description.m_bPreferDepthRangeZeroToOne = true;

      auto pDevice = spRHIImplementationFactory::CreateDevice(szRendererName, ezDefaultAllocatorWrapper::GetAllocator(), description);
      pRenderSystem->Startup(pDevice);
#else
      EZ_ASSERT_ALWAYS(false, "Direct3D11 is not supported on this platform.");
#endif
    }
    else if (eApiType == spGraphicsApi::Metal)
    {
#if EZ_ENABLED(EZ_PLATFORM_OSX)
      spRenderingSurfaceNSWindow renderSurface(m_pWindow->GetNativeWindowHandle());

      spDeviceDescription description{};
      description.m_bDebug = true;
      description.m_bSyncV = false;
      description.m_bUsSrgbFormat = true;
      description.m_uiWidth = g_uiWindowWidth;
      description.m_uiHeight = g_uiWindowHeight;
      description.m_bHasMainSwapchain = true;
      description.m_MainSwapchainDescription.m_uiWidth = g_uiWindowWidth;
      description.m_MainSwapchainDescription.m_uiHeight = g_uiWindowHeight;
      description.m_MainSwapchainDescription.m_bUseSrgb = false;
      description.m_MainSwapchainDescription.m_bVSync = false;
      description.m_MainSwapchainDescription.m_bUseDepthTexture = true;
      description.m_MainSwapchainDescription.m_pRenderingSurface = &renderSurface;
      description.m_MainSwapchainDescription.m_eDepthFormat = spPixelFormat::D32FloatS8UInt;
      description.m_bMainSwapchainHasDepth = true;
      description.m_eSwapchainDepthPixelFormat = spPixelFormat::D32FloatS8UInt;
      description.m_bPreferDepthRangeZeroToOne = true;

      auto pDevice = spRHIImplementationFactory::CreateDevice(szRendererName, ezDefaultAllocatorWrapper::GetAllocator(), description);
      pRenderSystem->Startup(pDevice);
#else
      EZ_ASSERT_ALWAYS(false, "Metal is not supported on this platform.");
#endif
    }

    EZ_ASSERT_DEV(pRenderSystem->GetDevice() != nullptr, "Device creation failed");

    pRenderSystem->GetCompositor()->SetViewportSize({g_uiWindowWidth, g_uiWindowHeight});
    pRenderSystem->GetCompositor()->SetRenderSize({g_uiWindowWidth, g_uiWindowHeight});
    pRenderSystem->GetCompositor()->SetEnableHDR(false);

    pRenderSystem->GetCompositor()->CreateCameraSlot("Main");
    pRenderSystem->GetCompositor()->m_pGameRenderer = EZ_DEFAULT_NEW(spSceneRenderer);
    static_cast<spSceneRenderer*>(pRenderSystem->GetCompositor()->m_pGameRenderer)->SetChildRenderer(EZ_DEFAULT_NEW(spCameraRenderer));
    static_cast<spCameraRenderer*>(static_cast<spSceneRenderer*>(pRenderSystem->GetCompositor()->m_pGameRenderer)->GetChildRenderer())->SetCameraSlot("Main");
    m_pRenderer = EZ_DEFAULT_NEW(spSequenceRenderer);
    // spClearRenderer* pClearRenderer = EZ_DEFAULT_NEW(spClearRenderer);
    // pClearRenderer->SetClearFlags(spClearRenderer::ClearFlags::Default);
    // pClearRenderer->SetClearDepth(1.0f);
    // pClearRenderer->SetClearStencil(0);
    // static_cast<spSequenceRenderer*>(m_pRenderer.Borrow())->PushBack(pClearRenderer);
    static_cast<spSequenceRenderer*>(m_pRenderer.Borrow())->PushBack(EZ_DEFAULT_NEW(spDeferredRenderer));
    // m_pRenderer = EZ_DEFAULT_NEW(spClearRenderer);
    // static_cast<spClearRenderer*>(m_pRenderer.Borrow())->SetClearFlags(spClearRenderer::ClearFlags::Default);
    // static_cast<spClearRenderer*>(m_pRenderer.Borrow())->SetClearDepth(1.0f);
    // static_cast<spClearRenderer*>(m_pRenderer.Borrow())->SetClearStencil(0);
    static_cast<spCameraRenderer*>(static_cast<spSceneRenderer*>(pRenderSystem->GetCompositor()->m_pGameRenderer)->GetChildRenderer())->SetChildRenderer(m_pRenderer.Borrow());
  }
}

void spRPIGameApplication::Deinit_ShutdownGraphicsDevice()
{
  // Release all RHI resources
  spRenderSystem::GetSingleton()->GetDevice()->GetResourceManager()->ReleaseResources();

  spRenderSystem::GetSingleton()->UnregisterSceneForWorld(m_pWorld.Borrow());
  m_pSceneContext = nullptr;

  spRenderSystem::GetSingleton()->Shutdown();

  m_pWindow->Destroy().IgnoreResult();
  m_pWindow = nullptr;
}

ezApplication::Execution spRPIGameApplication::Run()
{
  m_pWindow->ProcessWindowMessages();

  if (m_pWindow->m_bCloseRequested)
    return Execution::Quit;

  return SUPER::Run();
}

bool spRPIGameApplication::IsGameUpdateEnabled() const
{
  return true;
}

bool spRPIGameApplication::Run_ProcessApplicationInput()
{
  EZ_LOCK(m_pWorld->GetWriteMarker());

  if (ezInputManager::GetInputActionState("Main", "MoveCamBack") != ezKeyState::Up)
  {
    m_pCamera->SetLocalPosition(m_pCamera->GetLocalPosition() - ezVec3::MakeAxisX());
  }

  if (ezInputManager::GetInputActionState("Main", "MoveCamForward") != ezKeyState::Up)
  {
    m_pCamera->SetLocalPosition(m_pCamera->GetLocalPosition() + ezVec3::MakeAxisX());
  }

  if (ezInputManager::GetInputActionState("Main", "MoveCamLeft") != ezKeyState::Up)
  {
    m_pCamera->SetLocalPosition(m_pCamera->GetLocalPosition() - ezVec3::MakeAxisY());
  }

  if (ezInputManager::GetInputActionState("Main", "MoveCamRight") != ezKeyState::Up)
  {
    m_pCamera->SetLocalPosition(m_pCamera->GetLocalPosition() + ezVec3::MakeAxisY());
  }

  return SUPER::Run_ProcessApplicationInput();
}

void spRPIGameApplication::Run_WorldUpdateAndRender()
{
  EZ_PROFILE_SCOPE("Run_WorldUpdateAndRender");

  m_pSceneContext->Collect();

  {
    Run_BeforeWorldUpdate();
    {
      EZ_LOCK(m_pWorld->GetWriteMarker());
      m_pWorld->Update();
    }
    Run_AfterWorldUpdate();

    Run_UpdatePlugins();
  }

  if (spRenderSystem::IsMultiThreadedRendering())
    spRenderSystem::GetSingleton()->GetRenderThread()->PostAsync(ezMakeDelegate(&spSceneContext::BeginFrame, m_pSceneContext.Borrow()));
  else
    m_pSceneContext->BeginFrame();

  if (spRenderSystem::IsMultiThreadedRendering())
    spRenderSystem::GetSingleton()->GetRenderThread()->PostAsync(ezMakeDelegate(&spSceneContext::Draw, m_pSceneContext.Borrow()));
  else
    m_pSceneContext->Draw();
}

void spRPIGameApplication::Run_PresentImage()
{
  EZ_PROFILE_SCOPE("Run_PresentImage");
  if (spRenderSystem::IsMultiThreadedRendering())
    spRenderSystem::GetSingleton()->GetRenderThread()->PostAsync(ezMakeDelegate(&spSceneContext::Present, m_pSceneContext.Borrow()));
  else
    m_pSceneContext->Present();
}

void spRPIGameApplication::Run_FinishFrame()
{
  if (spRenderSystem::IsMultiThreadedRendering())
    spRenderSystem::GetSingleton()->GetRenderThread()->PostAsync(ezMakeDelegate(&spSceneContext::EndFrame, m_pSceneContext.Borrow()));
  else
    m_pSceneContext->EndFrame();

  m_pSceneContext->WaitForIdle();

  ezGameApplicationBase::Run_FinishFrame();
}

EZ_APPLICATION_ENTRY_POINT(spRPIGameApplication);
