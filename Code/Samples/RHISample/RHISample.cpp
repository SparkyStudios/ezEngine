#include <Core/Input/InputManager.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Core/System/Window.h>

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Time/Clock.h>

#include <RHI/CommandList.h>
#include <RHI/Core.h>
#include <RHI/Factory.h>
#include <RHI/Framebuffer.h>
#include <RHI/Profiler.h>
#include <RHI/ResourceSet.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/ResourceManager.h>
#include <RHID3D11/Swapchain.h>

#include <RHISample/RHISample.h>

static ezUInt32 g_uiWindowWidth = 640;
static ezUInt32 g_uiWindowHeight = 480;

class ezRHISampleWindow : public ezWindow
{
public:
  ezRHISampleWindow(ezRHISampleApp* pApp)
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
  ezRHISampleApp* m_pApp = nullptr;
};

ezRHISampleApp::ezRHISampleApp()
  : ezApplication("RHI Sample")
{
}

void ezRHISampleApp::AfterCoreSystemsStartup()
{
  ezStringBuilder sProjectDir = ">sdk/Data/Samples/RHISample";
  ezStringBuilder sProjectDirResolved;
  ezFileSystem::ResolveSpecialDirectory(sProjectDir, sProjectDirResolved).IgnoreResult();

  ezFileSystem::SetSpecialDirectory("project", sProjectDirResolved);

  ezFileSystem::AddDataDirectory("", "", ":", ezFileSystem::AllowWrites).IgnoreResult();
  ezFileSystem::AddDataDirectory(">appdir/", "AppBin", "bin", ezFileSystem::AllowWrites).IgnoreResult();                              // writing to the binary directory
  ezFileSystem::AddDataDirectory(">appdir/", "ShaderCache", "shadercache", ezFileSystem::AllowWrites).IgnoreResult();                 // for shader files
  ezFileSystem::AddDataDirectory(">user/ezEngine Project/RHISample", "AppData", "appdata", ezFileSystem::AllowWrites).IgnoreResult(); // app user data

  ezFileSystem::AddDataDirectory(">sdk/Data/Base", "Base", "base").IgnoreResult();
  ezFileSystem::AddDataDirectory(">project/", "Project", "project", ezFileSystem::AllowWrites).IgnoreResult();

  ezGlobalLog::AddLogWriter(ezLogWriter::Console::LogMessageHandler);
  ezGlobalLog::AddLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);

  ezPlugin::LoadPlugin("ezInspectorPlugin").IgnoreResult();

  // Register Input
  {
    ezInputActionConfig cfg;

    cfg = ezInputManager::GetInputActionConfig("Main", "CloseApp");
    cfg.m_sInputSlotTrigger[0] = ezInputSlot_KeyEscape;
    ezInputManager::SetInputActionConfig("Main", "CloseApp", cfg, true);
  }

  ezEnum<spGraphicsApi> eApiType = spGraphicsApi::Direct3D11;

  const char* szRendererName = ezCommandLineUtils::GetGlobalInstance()->GetStringOption("-rhi", 0, "D3D11");
  {
    if (ezStringUtils::Compare(szRendererName, "D3D11") == 0)
    {
      eApiType = spGraphicsApi::Direct3D11;
    }

    else if (ezStringUtils::Compare(szRendererName, "D3D12") == 0)
    {
      eApiType = spGraphicsApi::Direct3D12;
    }

    else if (ezStringUtils::Compare(szRendererName, "VK") == 0)
    {
      eApiType = spGraphicsApi::Vulkan;
    }

    else if (ezStringUtils::Compare(szRendererName, "GL") == 0)
    {
      eApiType = spGraphicsApi::OpenGL;
    }

    else if (ezStringUtils::Compare(szRendererName, "GLES") == 0)
    {
      eApiType = spGraphicsApi::OpenGLES;
    }

    else if (ezStringUtils::Compare(szRendererName, "MTL") == 0)
    {
      eApiType = spGraphicsApi::Metal;
    }
  }

  // Create a window for rendering
  {
    ezWindowCreationDesc WindowCreationDesc;
    WindowCreationDesc.m_Resolution.width = g_uiWindowWidth;
    WindowCreationDesc.m_Resolution.height = g_uiWindowHeight;
    WindowCreationDesc.m_Title = ezStringBuilder("RHISample ", szRendererName);
    WindowCreationDesc.m_bShowMouseCursor = true;
    WindowCreationDesc.m_bClipMouseCursor = false;
    WindowCreationDesc.m_WindowMode = ezWindowMode::WindowResizable;
    m_pWindow = EZ_DEFAULT_NEW(ezRHISampleWindow, this);
    m_pWindow->Initialize(WindowCreationDesc).IgnoreResult();
  }

  // Init the rendering thread
  {
    m_pRenderingThread = EZ_DEFAULT_NEW(spRenderingThread);
    m_pRenderingThread->Start();
  }

  // now that we have a window and device, tell the engine to initialize the rendering infrastructure
  ezStartup::StartupHighLevelSystems();

  if (eApiType == spGraphicsApi::Direct3D11)
  {
    spRenderingSurfaceWin32 renderSurface(ezMinWindows::ToNative(m_pWindow->GetNativeWindowHandle()), nullptr, m_pWindow->IsFullscreenWindow());

    spDeviceDescriptionD3D11 description;
    description.m_bDebug = false;
    description.m_bHasMainSwapchain = false;
    description.m_bSyncV = false;
    description.m_bUsSrgbFormat = true;
    description.m_uiWidth = g_uiWindowWidth;
    description.m_uiHeight = g_uiWindowHeight;
    description.m_bHasMainSwapchain = true;
    description.m_MainSwapchainDescription.m_uiWidth = g_uiWindowWidth;
    description.m_MainSwapchainDescription.m_uiHeight = g_uiWindowHeight;
    description.m_MainSwapchainDescription.m_bUseSrgb = true;
    description.m_MainSwapchainDescription.m_bVSync = false;
    description.m_MainSwapchainDescription.m_bUseDepthTexture = true;
    description.m_MainSwapchainDescription.m_pRenderingSurface = &renderSurface;
    description.m_MainSwapchainDescription.m_eDepthFormat = spPixelFormat::D24UNormS8UInt;
    description.m_bMainSwapchainHasDepth = true;
    description.m_eSwapchainDepthPixelFormat = spPixelFormat::D24UNormS8UInt;
    description.m_bPreferDepthRangeZeroToOne = true;

    device = spRHIImplementationFactory::CreateDevice(szRendererName, ezDefaultAllocatorWrapper::GetAllocator(), description);
  }

  EZ_ASSERT_DEV(device != nullptr, "Device creation failed");

  auto* pFactory = device->GetResourceFactory();

  ezUInt16 IndexBuffer[3] = {0, 1, 2};
  ibo = pFactory->CreateBuffer(spBufferDescription(sizeof(ezUInt16) * 3, spBufferUsage::IndexBuffer));
  ibo->SetDebugName("ibo");

  ezVec3 VertexBuffer[3] = {
    ezVec3(-0.5f, -0.5f, 0.0f),
    ezVec3(0.0f, 0.5f, 0.0f),
    ezVec3(0.5f, -0.5f, 0.0f)};
  vbo = pFactory->CreateBuffer(spBufferDescription(sizeof(ezVec3) * 3, spBufferUsage::VertexBuffer));
  vbo->SetDebugName("vbo");

  device->UpdateBuffer(ibo->GetHandle(), 0, reinterpret_cast<void*>(&IndexBuffer), sizeof(IndexBuffer));
  device->UpdateBuffer(vbo->GetHandle(), 0, reinterpret_cast<void*>(&VertexBuffer), sizeof(VertexBuffer));

  char vs_content[] = R"(
struct VS_INPUT
{
  float3 pos : POSITION;
};

struct VS_OUTPUT
{
  float4 pos: SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
  VS_OUTPUT output;
  output.pos = float4(input.pos, 1.0f);
  return output;
}
)";

  char ps_content[] = R"(
struct VS_OUTPUT
{
  float4 pos: SV_POSITION;
};

cbuffer Settings : register(b0)
{
  float4 color;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
  return color;
}
)";

  spShaderDescription vsDescription;
  vsDescription.m_sEntryPoint = ezMakeHashedString("main");
  vsDescription.m_eShaderStage = spShaderStage::VertexShader;
  vsDescription.m_Buffer = ezMakeByteArrayPtr<char>(vs_content, static_cast<ezUInt32>(sizeof(vs_content)));

  vs = pFactory->CreateShader(vsDescription);
  vs->SetDebugName("vs");

  spShaderDescription psDescription;
  psDescription.m_sEntryPoint = ezMakeHashedString("main");
  psDescription.m_eShaderStage = spShaderStage::PixelShader;
  psDescription.m_Buffer = ezMakeByteArrayPtr<char>(ps_content, static_cast<ezUInt32>(sizeof(ps_content)));

  ps = pFactory->CreateShader(psDescription);
  ps->SetDebugName("ps");

  spo = pFactory->CreateShaderProgram();
  spo->Attach(vs->GetHandle());
  spo->Attach(ps->GetHandle());
  spo->SetDebugName("spo");

  spInputLayoutDescription inputLayoutDescription;
  inputLayoutDescription.m_uiInstanceStepRate = 0;
  inputLayoutDescription.m_uiStride = sizeof(ezVec3);
  inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("pos", spInputElementLocationSemantic::Position, spInputElementFormat::Float3, 0));

  input = pFactory->CreateInputLayout(inputLayoutDescription, vs->GetHandle());
  input->SetDebugName("input");

  spResourceLayoutDescription resourceLayoutDescription;

  spResourceLayoutElementDescription rl1;
  rl1.m_eShaderStage = spShaderStage::PixelShader;
  rl1.m_eType = spShaderResourceType::ConstantBuffer;
  rl1.m_eOptions = spResourceLayoutElementOptions::None;
  rl1.m_sName = ezMakeHashedString("Settings");
  resourceLayoutDescription.m_Elements.PushBack(rl1);

  layout = pFactory->CreateResourceLayout(resourceLayoutDescription);
  layout->SetDebugName("layout");

  spBufferDescription cboDescription(sizeof(ezColor), spBufferUsage::ConstantBuffer | spBufferUsage::Dynamic | spBufferUsage::TripleBuffered);
  cbo = pFactory->CreateBuffer(cboDescription);
  cbo->SetDebugName("cbo");

  spResourceSetDescription setDesc;
  setDesc.m_hResourceLayout = layout->GetHandle();
  setDesc.m_BoundResources.PushBack(cbo->GetHandle());
  set = pFactory->CreateResourceSet(setDesc);
  set->SetDebugName("set");

  const auto pSwapchain = device->GetMainSwapchain();

  spGraphicPipelineDescription desc;
  desc.m_Output = pSwapchain->GetFramebuffer()->GetOutputDescription();
  desc.m_ePrimitiveTopology = spPrimitiveTopology::Triangles;
  desc.m_RenderingState.m_BlendState = spBlendState::SingleDisabled;
  desc.m_RenderingState.m_DepthState = spDepthState::Disabled;
  desc.m_RenderingState.m_RasterizerState = spRasterizerState::Default;
  desc.m_RenderingState.m_StencilState = spStencilState::Disabled;
  desc.m_ShaderPipeline.m_hShaderProgram = spo->GetHandle();
  desc.m_ShaderPipeline.m_InputLayouts.PushBack(input->GetHandle());
  desc.m_ResourceLayouts.PushBack(layout->GetHandle());

  gpo = pFactory->CreateGraphicPipeline(desc);
  gpo->SetDebugName("gpo");
};

void ezRHISampleApp::BeforeHighLevelSystemsShutdown()
{
  // tell the engine that we are about to destroy window and graphics device,
  // and that it therefore needs to cleanup anything that depends on that
  ezStartup::ShutdownHighLevelSystems();

  device->GetResourceManager()->ReleaseResources();

  gpo.Clear();
  set.Clear();
  layout.Clear();
  input.Clear();
  spo.Clear();
  ps.Clear();
  vs.Clear();
  ibo.Clear();
  vbo.Clear();
  cbo.Clear();

  // destroy device
  device->Destroy();

  m_pRenderingThread->Stop();
  EZ_DEFAULT_DELETE(m_pRenderingThread);

  // finally destroy the window
  m_pWindow->Destroy().IgnoreResult();
  EZ_DEFAULT_DELETE(m_pWindow);
}

void ezRHISampleApp::OnResize(ezUInt32 width, ezUInt32 height)
{
  device->ResizeSwapchain(width, height);
}

ezApplication::Execution ezRHISampleApp::Run()
{
  m_pWindow->ProcessWindowMessages();

  if (m_pWindow->m_bCloseRequested || ezInputManager::GetInputActionState("Main", "CloseApp") == ezKeyState::Pressed)
    return Execution::Quit;

  // make sure time goes on
  ezClock::GetGlobalClock()->Update();

  // update all input state
  ezInputManager::Update(ezClock::GetGlobalClock()->GetTimeDiff());

  ezSharedPtr<spScopeProfiler> pTestScopeProfiler;

  // do the rendering
  device->BeginFrame();
  {
    const auto pSwapchain = device->GetMainSwapchain();

    const auto commandList = device->GetResourceFactory()->CreateCommandList(spCommandListDescription());
    commandList->SetDebugName("RHI SAMPLE CMD");

    const auto c = ezAngle::Radian(ezTime::Now().AsFloatInSeconds());
    const auto color = ezColor(ezMath::Sin(c), ezMath::Cos(c), ezMath::Sin(-c), 1.0f);

    commandList->Begin();
    {
      commandList->PushProfileScope("Test");
      {
        commandList->SetGraphicPipeline(gpo->GetHandle());

        commandList->SetFramebuffer(pSwapchain->GetFramebuffer()->GetHandle());
        commandList->SetFullViewport(0);
        commandList->ClearColorTarget(0, ezColor::Black);
        commandList->ClearDepthStencilTarget(1.0f, 0);

        commandList->SetVertexBuffer(0, vbo->GetHandle());
        commandList->SetIndexBuffer(ibo->GetHandle(), spIndexFormat::UInt16);

        commandList->UpdateBuffer<ezColor>(cbo->GetHandle(), 0, &color, 1);

        commandList->SetGraphicResourceSet(0, set->GetHandle());

        commandList->DrawIndexed(3, 1, 0, 0, 0);
      }
      commandList->PopProfileScope(pTestScopeProfiler);
    }
    commandList->End();

    device->SubmitCommandList(commandList->GetHandle());
  }
  device->EndFrame();

  // Swap buffers - This will use the next buffer range for the next rendering pass
  cbo->SwapBuffers();

  if (device->IsDebugEnabled())
    ezLog::Info("RHI Sample: Frame {0} Time: {1}ms", device->GetFrameCount(), (pTestScopeProfiler->GetEndTime() - pTestScopeProfiler->GetBeginTime()).AsFloatInSeconds() * 1000);

  // needs to be called once per frame
  ezResourceManager::PerFrameUpdate();

  // tell the task system to finish its work for this frame
  // this has to be done at the very end, so that the task system will only use up the time that is left in this frame for
  // uploading GPU data etc.
  ezTaskSystem::FinishFrameTasks();

  return ezApplication::Execution::Continue;
}

EZ_CONSOLEAPP_ENTRY_POINT(ezRHISampleApp);
