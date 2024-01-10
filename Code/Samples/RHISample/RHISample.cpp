#include <Core/Assets/AssetFileHeader.h>
#include <Core/Input/InputManager.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Core/System/Window.h>

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Time/Clock.h>
#include <Foundation/Types/VariantTypeRegistry.h>

#include <Texture/Image/Image.h>

#include <RAI/Resources/ImageResource.h>
#include <RAI/Resources/MeshResource.h>
#include <RAI/Resources/SamplerResource.h>
#include <RAI/Resources/Texture2DResource.h>

#include <RHI/CommandList.h>
#include <RHI/Core.h>
#include <RHI/Factory.h>
#include <RHI/Framebuffer.h>
#include <RHI/Profiler.h>
#include <RHI/ResourceSet.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)
#  include <RHID3D11/Device.h>
#  include <RHID3D11/ResourceManager.h>
#  include <RHID3D11/Swapchain.h>
#elif EZ_ENABLED(EZ_PLATFORM_OSX)
#  include <RHIMTL/Device.h>
#  include <RHIMTL/Swapchain.h>
#endif

#include <RPI/Graph/Nodes/MainSwapchainRenderGraphNode.h>

#include <RHISample/RHISample.h>

typedef spTriangleDemoRenderGraphNode::PassData spTriangleDemoRenderGraphNodePassData;

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(spTriangleDemoRenderGraphNodePassData, ezNoBase, 1, ezRTTIDefaultAllocator<spTriangleDemoRenderGraphNodePassData>)
EZ_END_STATIC_REFLECTED_TYPE;

EZ_DEFINE_CUSTOM_VARIANT_TYPE(spTriangleDemoRenderGraphNodePassData);
// clang-format on

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
  ezFileSystem::AddDataDirectory(">sdk/Data/Content", "Content", "content").IgnoreResult();
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
    WindowCreationDesc.m_Title = ezStringBuilder("RHISample ", szRendererName);
    WindowCreationDesc.m_bShowMouseCursor = true;
    WindowCreationDesc.m_bClipMouseCursor = false;
    WindowCreationDesc.m_WindowMode = ezWindowMode::WindowResizable;
    m_pWindow = EZ_DEFAULT_NEW(ezRHISampleWindow, this);
    m_pWindow->Initialize(WindowCreationDesc).IgnoreResult();
  }

  // Init the rendering system
  m_pRenderSystem = EZ_DEFAULT_NEW(RPI::spRenderSystem);

  // now that we have a window and device, tell the engine to initialize the rendering infrastructure
  ezStartup::StartupHighLevelSystems();

  if (eApiType == spGraphicsApi::Direct3D11)
  {
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)
    spRenderingSurfaceWin32 renderSurface(ezMinWindows::ToNative(m_pWindow->GetNativeWindowHandle()), nullptr, m_pWindow->IsFullscreenWindow());

    spDeviceDescriptionD3D11 description{};
    description.m_bDebug = true;
    description.m_bHasMainSwapchain = false;
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
    m_pRenderSystem->Startup(pDevice);
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
    description.m_bHasMainSwapchain = false;
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
    m_pRenderSystem->Startup(pDevice);
#else
    EZ_ASSERT_ALWAYS(false, "Metal is not supported on this platform.");
#endif
  }

  EZ_ASSERT_DEV(m_pRenderSystem->GetDevice() != nullptr, "Device creation failed");

  auto* pFactory = m_pRenderSystem->GetDevice()->GetResourceFactory();

  m_hTexture = ezResourceManager::LoadResource<spTexture2DResource>(":project/textures/grid.spTexture2D");

  const ezResourceLock imageResource(m_hTexture, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (!imageResource.IsValid())
    return ezLog::Error("Unable to get the image resource! Make sure to run AssetProcessor first.");

  m_hMesh = ezResourceManager::LoadResource<spMeshResource>(":project/objects/yemaya_body.spMesh");
  const ezResourceLock resource(m_hMesh, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (!resource.IsValid())
    return ezLog::Error("Unable to get the mesh resource! Make sure to run AssetProcessor first.");

  auto& mesh = resource.GetPointerNonConst()->GetLOD(0);
  m_pMesh = &mesh;

  mesh.CreateRHIVertexBuffer();
  mesh.CreateRHIIndexBuffer();
  mesh.CreateRHIIndirectBuffer();

  // --- Begin Experimental render graph

  // 1. Setup graph
  graphBuilder = EZ_NEW(m_pRenderSystem->GetDevice()->GetAllocator(), spRenderGraphBuilder, m_pRenderSystem->GetDevice().Borrow());

  spResourceHandle importedTex = graphBuilder->Import(imageResource.GetPointer()->GetRHITexture()); // Import a resource in the graph
  spResourceHandle importedSmp = graphBuilder->Import(imageResource.GetPointer()->GetRHISampler()); // Import a resource in the graph
  spResourceHandle importedVBO = graphBuilder->Import(m_pMesh->GetRHIVertexBuffer());               // Import a resource in the graph
  spResourceHandle importedIBO = graphBuilder->Import(m_pMesh->GetRHIIndexBuffer());                // Import a resource in the graph
  spResourceHandle importedIDB = graphBuilder->Import(m_pMesh->GetRHIIndirectBuffer());             // Import a resource in the graph

  // 2. Setup graph nodes
  ezUniquePtr<spTriangleDemoRenderGraphNode> triangleNode = EZ_NEW(m_pRenderSystem->GetDevice()->GetAllocator(), spTriangleDemoRenderGraphNode, m_pMesh);

  ezHashTable<ezHashedString, spResourceHandle> triangleNodeResources;
  triangleNodeResources.Insert(ezMakeHashedString("tex"), importedTex);
  triangleNodeResources.Insert(ezMakeHashedString("smp"), importedSmp);
  triangleNodeResources.Insert(ezMakeHashedString("vbo"), importedVBO);
  triangleNodeResources.Insert(ezMakeHashedString("ibo"), importedIBO);
  triangleNodeResources.Insert(ezMakeHashedString("idb"), importedIDB);

  graphBuilder->AddNode("Triangle", std::move(triangleNode), triangleNodeResources);

  ezUniquePtr<spMainSwapchainRenderGraphNode> swapchainNode = EZ_NEW(m_pRenderSystem->GetDevice()->GetAllocator(), spMainSwapchainRenderGraphNode);
  swapchainNode->SetRenderTargetSize(g_uiWindowWidth, g_uiWindowHeight);

  ezHashTable<ezHashedString, spResourceHandle> swapchainNodeResources;
  swapchainNodeResources.Insert(ezMakeHashedString("Input"), static_cast<const spTriangleDemoRenderGraphNode*>(graphBuilder->GetNode("Triangle"))->GetTarget());

  graphBuilder->AddNode("Swapchain", std::move(swapchainNode), swapchainNodeResources);

  // 3. Compile graph
  renderPipeline = graphBuilder->Compile();

  // --- End Experimental render graph


  m_pRenderSystem->GetSceneContext()->AddPipeline(renderPipeline.Borrow());
};

void ezRHISampleApp::BeforeHighLevelSystemsShutdown()
{
  // tell the engine that we are about to destroy window and graphics device,
  // and that it therefore needs to cleanup anything that depends on that
  ezStartup::ShutdownHighLevelSystems();

  // release pending resources
  m_pRenderSystem->GetDevice()->GetResourceManager()->ReleaseResources();

  // cleanup the render pipeline
  renderPipeline->CleanUp();

  m_hMesh.Invalidate();

  m_hTexture.Invalidate();

  renderPipeline.Clear();
  graphBuilder.Clear();
}

void ezRHISampleApp::AfterCoreSystemsShutdown()
{
  m_pRenderSystem->Shutdown();
  m_pRenderSystem = nullptr;

  // finally destroy the window
  m_pWindow->Destroy().IgnoreResult();
  EZ_DEFAULT_DELETE(m_pWindow);
}

void ezRHISampleApp::OnResize(ezUInt32 width, ezUInt32 height)
{
  m_pRenderSystem->GetDevice()->ResizeSwapchain(width, height);
}

ezResult spTriangleDemoRenderGraphNode::Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources)
{
  if (!resources.TryGetValue("tex", m_PassData.m_hGridTexture))
    return EZ_FAILURE;
  if (!resources.TryGetValue("smp", m_PassData.m_hSampler))
    return EZ_FAILURE;
  if (!resources.TryGetValue("vbo", m_PassData.m_hVertexBuffer))
    return EZ_FAILURE;
  if (!resources.TryGetValue("ibo", m_PassData.m_hIndexBuffer))
    return EZ_FAILURE;
  if (!resources.TryGetValue("idb", m_PassData.m_hIndirectBuffer))
    return EZ_FAILURE;

  m_pMesh->GetDrawCommands(m_PassData.m_DrawCommands);

  spRenderTargetDescription rtDescription{};
  rtDescription.m_bGenerateMipMaps = false;
  rtDescription.m_eQuality = spRenderTargetQuality::LDR;
  rtDescription.m_eSampleCount = spTextureSampleCount::None;
  rtDescription.m_uiHeight = g_uiWindowHeight;
  rtDescription.m_uiWidth = g_uiWindowWidth;

  m_PassData.m_hGridTexture = pBuilder->Read(this, m_PassData.m_hGridTexture);
  m_PassData.m_hConstantBuffer = pBuilder->CreateBuffer(this, spBufferDescription(sizeof(ezColor), spBufferUsage::ConstantBuffer | spBufferUsage::Dynamic | spBufferUsage::TripleBuffered), spRenderGraphResourceBindType::Transient);
  m_PassData.m_hIndexBuffer = pBuilder->Read(this, m_PassData.m_hIndexBuffer);
  m_PassData.m_hVertexBuffer = pBuilder->Read(this, m_PassData.m_hVertexBuffer);
  m_PassData.m_hIndirectBuffer = pBuilder->Read(this, m_PassData.m_hIndirectBuffer);
  m_PassData.m_hSampler = pBuilder->Read(this, m_PassData.m_hSampler);
  m_PassData.m_hRenderTarget = pBuilder->CreateRenderTarget(this, rtDescription, spRenderGraphResourceBindType::ReadOnly);

  return EZ_SUCCESS;
}

ezUniquePtr<spRenderPass> spTriangleDemoRenderGraphNode::Compile(spRenderGraphBuilder* pBuilder)
{
  auto& data = m_PassData;
  const auto& resources = pBuilder->GetResources();

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  constexpr ezUInt8 szVertexShader[] = R"(
struct VS_INPUT
{
  float3 pos : POSITION;
  float3 nrm : Normal;
  float3 tnt : Tangent;
  float3 btt : BiTangent;
  float2 uv0 : TexCoord0;
  float4 bw0 : BoneWeights0;
  uint4 bi1 : BoneIndices0;
};

struct VS_OUTPUT
{
  float4 pos : SV_POSITION;
  float2 uv0 : TEXCOORD0;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
  VS_OUTPUT output;
  output.pos = float4(input.pos, 1.0f);
  output.uv0 = input.uv0;
  return output;
}
)";

  constexpr ezUInt8 szPixelShader[] = R"(
struct VS_OUTPUT
{
  float4 pos : SV_POSITION;
  float2 uv0 : TEXCOORD0;
};

SamplerState linearSampler : register(s0);

cbuffer Settings : register(b0)
{
  float4 color;
};

Texture2D<float4> tex : register(t0);

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
  return tex.SampleLevel(linearSampler, input.uv0, 0) * color;
}
)";
#elif EZ_ENABLED(EZ_PLATFORM_OSX)
  constexpr ezUInt8 szVertexShader[] = R"(
#include <metal_stdlib>
using namespace metal;

struct VS_INPUT
{
  float3 pos [[attribute(0)]];
  float3 nrm [[attribute(1)]];
  float3 tnt [[attribute(2)]];
  float3 btt [[attribute(3)]];
  float2 uv0 [[attribute(4)]];
  float4 bw0 [[attribute(5)]];
  uint4  bi1 [[attribute(6)]];
};

struct VS_OUTPUT {
    float4 pos [[position]];
    float2 uv0;
};

vertex VS_OUTPUT VSMain(VS_INPUT input [[stage_in]]) {
    VS_OUTPUT output;

    output.pos = float4(input.pos, 1.0f);
    output.uv0 = input.uv0;

    return output;
}
)";

  constexpr ezUInt8 szPixelShader[] = R"(
#include <metal_stdlib>
using namespace metal;

struct VS_OUTPUT {
    float4 pos [[position]];
    float2 uv0;
};

struct Settings {
    float4 color;
};

fragment float4 PSMain(VS_OUTPUT in [[stage_in]], texture2d<float> tex [[texture(0)]], sampler linearSampler [[sampler(0)]], constant Settings* settings [[buffer(0)]]) {
    // Sample the texture to obtain a color
    const float4 colorSample = tex.sample(linearSampler, in.uv0) * settings->color;
    return colorSample;
}
)";
#endif
  // constexpr ezUInt16 IndexBuffer[3] = {0, 1, 2};

  // constexpr float VertexBuffer[9] = {
  //   -0.5f, -0.5f, 0.0,
  //   0.0f, 0.5f, 0.0,
  //   0.5f, -0.5f, 0.0f};

  spRenderGraphResource* tex = nullptr;
  resources.TryGetValue(data.m_hGridTexture.GetInternalID(), tex);
  tex->m_pResource->SetDebugName("grid_texture");

  spRenderGraphResource* cbo = nullptr;
  resources.TryGetValue(data.m_hConstantBuffer.GetInternalID(), cbo);
  cbo->m_pResource->SetDebugName("color_buffer");

  spRenderGraphResource* ibo = nullptr;
  resources.TryGetValue(data.m_hIndexBuffer.GetInternalID(), ibo);

  spRenderGraphResource* vbo = nullptr;
  resources.TryGetValue(data.m_hVertexBuffer.GetInternalID(), vbo);

  spRenderGraphResource* smp = nullptr;
  resources.TryGetValue(data.m_hSampler.GetInternalID(), smp);
  smp->m_pResource->SetDebugName("linear_sampler");

  spRenderGraphResource* rtt = nullptr;
  resources.TryGetValue(data.m_hRenderTarget.GetInternalID(), rtt);
  rtt->m_pResource->SetDebugName("main_render_target");

  spRenderGraphResource* idb = nullptr;
  resources.TryGetValue(data.m_hIndirectBuffer.GetInternalID(), idb);

  if (data.m_pVertexShader == nullptr)
  {
    spShaderDescription vsDescription{};
    vsDescription.m_sEntryPoint = ezMakeHashedString("VSMain");
    vsDescription.m_eShaderStage = spShaderStage::VertexShader;
    vsDescription.m_Buffer = ezMakeByteArrayPtr(szVertexShader, static_cast<ezUInt32>(sizeof(szVertexShader)));

    data.m_pVertexShader = pBuilder->GetResourceFactory()->CreateShader(vsDescription);
    data.m_pVertexShader->SetDebugName("vs");
  }

  if (data.m_pPixelShader == nullptr)
  {
    spShaderDescription psDescription{};
    psDescription.m_sEntryPoint = ezMakeHashedString("PSMain");
    psDescription.m_eShaderStage = spShaderStage::PixelShader;
    psDescription.m_Buffer = ezMakeByteArrayPtr(szPixelShader, static_cast<ezUInt32>(sizeof(szPixelShader)));

    data.m_pPixelShader = pBuilder->GetResourceFactory()->CreateShader(psDescription);
    data.m_pPixelShader->SetDebugName("ps");
  }

  if (data.m_pShaderProgram == nullptr)
  {
    data.m_pShaderProgram = pBuilder->GetResourceFactory()->CreateShaderProgram();
    data.m_pShaderProgram->Attach(data.m_pVertexShader);
    data.m_pShaderProgram->Attach(data.m_pPixelShader);
    data.m_pShaderProgram->SetDebugName("spo");
  }

  if (data.m_pInputLayout == nullptr)
  {
    spInputLayoutDescription inputLayoutDescription{};
    m_pMesh->GetRHIInputLayoutDescription(inputLayoutDescription);
    //    inputLayoutDescription.m_uiInstanceStepRate = 0;
    //    inputLayoutDescription.m_uiStride = sizeof(spVertex);
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("pos", spInputElementLocationSemantic::Position, spInputElementFormat::Float3, 0));
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("nrm", spInputElementLocationSemantic::Normal, spInputElementFormat::Float3, offsetof(spVertex, m_vNormal)));
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("tnt", spInputElementLocationSemantic::Tangent, spInputElementFormat::Float4, offsetof(spVertex, m_vTangent)));
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("btt", spInputElementLocationSemantic::BiTangent, spInputElementFormat::Float4, offsetof(spVertex, m_vBiTangent)));
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("uv0", spInputElementLocationSemantic::TexCoord, spInputElementFormat::Float2, offsetof(spVertex, m_vTexCoord0)));
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("uv1", spInputElementLocationSemantic::TexCoord, spInputElementFormat::Float2, offsetof(spVertex, m_vTexCoord1)));
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("cl0", spInputElementLocationSemantic::Color, spInputElementFormat::Byte4, offsetof(spVertex, m_Color0)));
    //    inputLayoutDescription.m_Elements.PushBack(spInputElementDescription("cl1", spInputElementLocationSemantic::Color, spInputElementFormat::Byte4, offsetof(spVertex, m_Color1)));

    data.m_pInputLayout = pBuilder->GetResourceFactory()->CreateInputLayout(inputLayoutDescription, data.m_pVertexShader->GetHandle());
    data.m_pInputLayout->SetDebugName("input");
  }

  ezDynamicArray<spResourceLayoutDescription> layouts;
  data.m_pShaderProgram->GetResourceLayoutDescriptions(layouts);

  if (data.m_pResourceLayout == nullptr)
  {
    spResourceLayoutDescription resourceLayoutDescription{};

    spResourceLayoutElementDescription rl3{};
    rl3.m_eShaderStage = spShaderStage::PixelShader;
    rl3.m_eType = spShaderResourceType::Sampler;
    rl3.m_eOptions = spResourceLayoutElementOptions::None;
    rl3.m_sName = ezMakeHashedString("linearSampler");
    resourceLayoutDescription.m_Elements.PushBack(rl3);

    spResourceLayoutElementDescription rl2{};
    rl2.m_eShaderStage = spShaderStage::PixelShader;
    rl2.m_eType = spShaderResourceType::ReadOnlyTexture;
    rl2.m_eOptions = spResourceLayoutElementOptions::None;
    rl2.m_sName = ezMakeHashedString("tex");
    resourceLayoutDescription.m_Elements.PushBack(rl2);

    spResourceLayoutElementDescription rl1{};
    rl1.m_eShaderStage = spShaderStage::PixelShader;
    rl1.m_eType = spShaderResourceType::ConstantBuffer;
    rl1.m_eOptions = spResourceLayoutElementOptions::None;
    rl1.m_sName = ezMakeHashedString("Settings");
    resourceLayoutDescription.m_Elements.PushBack(rl1);

    data.m_pResourceLayout = pBuilder->GetResourceFactory()->CreateResourceLayout(resourceLayoutDescription);
    data.m_pResourceLayout->SetDebugName("layout");

    // data.m_pResourceLayout = pBuilder->GetResourceFactory()->CreateResourceLayout(layouts[0]);
    // data.m_pResourceLayout->SetDebugName("layout");
  }

  if (data.m_pResourceSet == nullptr)
  {
    spResourceSetDescription setDesc{};
    setDesc.m_hResourceLayout = data.m_pResourceLayout->GetHandle();

    setDesc.m_BoundResources.Insert(ezMakeHashedString("linearSampler"), smp->m_pResource->GetHandle());
    setDesc.m_BoundResources.Insert(ezMakeHashedString("tex"), tex->m_pResource->GetHandle());
    setDesc.m_BoundResources.Insert(ezMakeHashedString("settings"), cbo->m_pResource->GetHandle());

    data.m_pResourceSet = pBuilder->GetResourceFactory()->CreateResourceSet(setDesc);
    data.m_pResourceSet->SetDebugName("set");
  }

  if (data.m_pGraphicPipeline == nullptr)
  {
    spGraphicPipelineDescription desc{};
    desc.m_Output = rtt->m_pResource.Downcast<spRenderTarget>()->GetFramebuffer()->GetOutputDescription();
    desc.m_ePrimitiveTopology = spPrimitiveTopology::Triangles;
    desc.m_RenderingState.m_BlendState = spBlendState::SingleDisabled;
    desc.m_RenderingState.m_DepthState = spDepthState::Disabled;
    desc.m_RenderingState.m_RasterizerState = spRasterizerState::Default;
    desc.m_RenderingState.m_StencilState = spStencilState::Disabled;
    desc.m_ShaderPipeline.m_hShaderProgram = data.m_pShaderProgram->GetHandle();
    desc.m_ShaderPipeline.m_InputLayouts.PushBack(data.m_pInputLayout->GetHandle());
    desc.m_ResourceLayouts.PushBack(data.m_pResourceLayout->GetHandle());

    data.m_pGraphicPipeline = pBuilder->GetResourceFactory()->CreateGraphicPipeline(desc);
    data.m_pGraphicPipeline->SetDebugName("gpo");
  }

  const spRenderPass::ExecuteCallback executeCallback = [=](const spRenderGraphResourcesTable& resources, spRenderContext* context, ezVariant& passData) -> void
  {
    spRenderGraphResource* cbo = nullptr;
    resources.TryGetValue(data.m_hConstantBuffer.GetInternalID(), cbo);

    spRenderGraphResource* ibo = nullptr;
    resources.TryGetValue(data.m_hIndexBuffer.GetInternalID(), ibo);

    spRenderGraphResource* vbo = nullptr;
    resources.TryGetValue(data.m_hVertexBuffer.GetInternalID(), vbo);

    spRenderGraphResource* rtt = nullptr;
    resources.TryGetValue(data.m_hRenderTarget.GetInternalID(), rtt);

    const auto cl = context->GetCommandList();

    const auto c = ezAngle::MakeFromRadian(ezTime::Now().AsFloatInSeconds());
    const auto col = ezColor(ezMath::Sin(c), ezMath::Cos(c), ezMath::Sin(-c), 1.0f);

    ezSharedPtr<spScopeProfiler> pTestScopeProfiler;

    cl->PushProfileScope(GetName());
    {
      cl->SetGraphicPipeline(data.m_pGraphicPipeline);

      cl->SetFramebuffer(rtt->m_pResource.Downcast<spRenderTarget>()->GetFramebuffer());

      cl->SetFullViewport(0);
      cl->SetFullScissorRect(0);

      cl->ClearColorTarget(0, ezColor::Black);
      // cl->ClearDepthStencilTarget(1.0f, 0);

      cl->SetVertexBuffer(0, vbo->m_pResource.Downcast<spBuffer>());
      cl->SetIndexBuffer(ibo->m_pResource.Downcast<spBuffer>(), spIndexFormat::UInt16);

      cl->UpdateBuffer<ezColor>(cbo->m_pResource.Downcast<spBuffer>(), 0, col);

      cl->SetGraphicResourceSet(0, data.m_pResourceSet);

      cl->DrawIndexedIndirect(idb->m_pResource.Downcast<spBuffer>(), 0, m_PassData.m_DrawCommands.GetCount(), context->GetDevice()->GetIndexedIndirectCommandSize());
    }
    cl->PopProfileScope(pTestScopeProfiler);

    // Swap buffers - This will use the next buffer range for the next rendering pass
    cbo->m_pResource.Downcast<spBuffer>()->SwapBuffers();
  };

  const spRenderPass::CleanUpCallback cleanUpCallback = [](const spRenderGraphResourcesTable& resources, ezVariant& passData) -> void
  {
    auto& data = passData.GetWritable<PassData>();

    spRenderGraphResource* cbo = nullptr;
    if (resources.TryGetValue(data.m_hConstantBuffer.GetInternalID(), cbo))
      cbo->m_pResource.Clear();

    spRenderGraphResource* ibo = nullptr;
    resources.TryGetValue(data.m_hIndexBuffer.GetInternalID(), ibo);
    ibo->m_pResource.Clear();

    spRenderGraphResource* vbo = nullptr;
    resources.TryGetValue(data.m_hVertexBuffer.GetInternalID(), vbo);
    vbo->m_pResource.Clear();

    spRenderGraphResource* smp = nullptr;
    resources.TryGetValue(data.m_hSampler.GetInternalID(), smp);
    smp->m_pResource.Clear();

    spRenderGraphResource* rtt = nullptr;
    resources.TryGetValue(data.m_hRenderTarget.GetInternalID(), rtt);
    rtt->m_pResource.Clear();

    data.m_pGraphicPipeline.Clear();
    data.m_pResourceSet.Clear();
    data.m_pResourceLayout.Clear();
    data.m_pInputLayout.Clear();
    data.m_pShaderProgram.Clear();
    data.m_pPixelShader.Clear();
    data.m_pVertexShader.Clear();
  };

  ezUniquePtr<spRenderPass> pPass = EZ_NEW(pBuilder->GetAllocator(), spRenderPass, executeCallback, cleanUpCallback);
  pPass->SetData(m_PassData);

  return pPass;
}

bool spTriangleDemoRenderGraphNode::IsEnabled() const
{
  return true;
}

void operator<<(ezStreamWriter& Stream, const spTriangleDemoRenderGraphNode::PassData& Value)
{
}

void operator>>(ezStreamReader& Stream, spTriangleDemoRenderGraphNode::PassData& Value)
{
}

bool operator==(const spTriangleDemoRenderGraphNode::PassData& lhs, const spTriangleDemoRenderGraphNode::PassData& rhs)
{
  return lhs.m_hConstantBuffer == rhs.m_hConstantBuffer && lhs.m_hIndexBuffer == rhs.m_hIndexBuffer && lhs.m_hVertexBuffer == rhs.m_hVertexBuffer && lhs.m_hRenderTarget == rhs.m_hRenderTarget && lhs.m_hGridTexture == rhs.m_hGridTexture && lhs.m_hSampler == rhs.m_hSampler;
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

  // do the rendering
  m_pRenderSystem->GetRenderThread()->PostAsync([&]() -> void
    {
      m_pRenderSystem->GetSceneContext()->BeginFrame();
      {
        m_pRenderSystem->GetSceneContext()->Draw();
      }
      m_pRenderSystem->GetSceneContext()->EndFrame(); });

  m_pRenderSystem->GetSceneContext()->WaitForIdle();

  // needs to be called once per frame
  ezResourceManager::PerFrameUpdate();

  // tell the task system to finish its work for this frame
  // this has to be done at the very end, so that the task system will only use up the time that is left in this frame for
  // uploading GPU data etc.
  ezTaskSystem::FinishFrameTasks();

  return ezApplication::Execution::Continue;
}

EZ_CONSOLEAPP_ENTRY_POINT(ezRHISampleApp);
