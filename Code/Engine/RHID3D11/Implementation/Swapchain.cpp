#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/CommandList.h>
#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Framebuffer.h>
#include <RHID3D11/Swapchain.h>
#include <RHID3D11/Texture.h>

// clang-off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderingSurfaceWin32, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderingSurfaceUWP, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
#endif
// clang-on

void spSwapchainD3D11::ReleaseResource()
{
  if (m_pDepthTexture != nullptr)
  {
    m_pDevice->GetResourceManager()->ReleaseResource(m_pDepthTexture->GetHandle());
    m_pDepthTexture = nullptr;
  }

  if (m_pFramebuffer != nullptr)
  {
    m_pDevice->GetResourceManager()->ReleaseResource(m_pFramebuffer->GetHandle());
    m_pFramebuffer = nullptr;
  }

  SP_RHI_DX11_RELEASE(m_pD3D11SwapChain);
}

bool spSwapchainD3D11::IsReleased() const
{
  return m_pD3D11SwapChain == nullptr;
}

void spSwapchainD3D11::SetDebugName(const ezString& debugName)
{
  spSwapchain::SetDebugName(debugName);

  m_pD3D11SwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.GetElementCount(), debugName.GetData());
}

spResourceHandle spSwapchainD3D11::GetFramebuffer() const
{
  if (m_pFramebuffer != nullptr)
    return m_pFramebuffer->GetHandle();

  return {};
}

void spSwapchainD3D11::SetVSync(bool bVSync)
{
  m_bVSync = bVSync;
  m_uiSyncInterval = bVSync ? 1 : 0;
}

void spSwapchainD3D11::Resize(ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  {
    EZ_LOCK(m_CLsLock);

    for (auto it = m_DependentCommandLists.GetIterator(); it.IsValid(); it.Next())
      (*it)->Reset();

    m_DependentCommandLists.Clear();
  }

  bool bResizeBuffers = false;

  if (m_pFramebuffer != nullptr)
  {
    bResizeBuffers = true;

    if (m_pDepthTexture != nullptr)
      m_pDevice->GetResourceManager()->ReleaseResource(m_pDepthTexture->GetHandle());

    m_pDevice->GetResourceManager()->ReleaseResource(m_pFramebuffer->GetHandle());
  }

  const auto uiActualWidth = static_cast<ezUInt32>(uiWidth * m_fPixelScale);
  const auto uiActualHeight = static_cast<ezUInt32>(uiHeight * m_fPixelScale);

  if (bResizeBuffers)
  {
    const HRESULT res = m_pD3D11SwapChain->ResizeBuffers(2, uiActualWidth, uiActualHeight, m_eColorFormat, 0);
    EZ_HRESULT_TO_LOG(res);
  }

  {
    spD3D11ScopedResource<ID3D11Texture2D> pBackBufferTexture;
    m_pD3D11SwapChain->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void**>(&pBackBufferTexture));

    if (m_Description.m_bUseDepthTexture)
    {
      spTextureDescription desc;
      desc.m_uiWidth = uiActualWidth;
      desc.m_uiHeight = uiActualHeight;
      desc.m_uiArrayLayers = 1;
      desc.m_uiMipCount = 1;
      desc.m_uiDepth = 1;
      desc.m_eFormat = m_Description.m_eDepthFormat;
      desc.m_eUsage = spTextureUsage::DepthStencil;
      desc.m_eDimension = spTextureDimension::Texture2D;

      m_pDevice->GetResourceManager()->ReleaseResource(m_pDepthTexture->GetHandle());
      m_pDepthTexture = new spTextureD3D11(ezStaticCast<spDeviceD3D11*>(m_pDevice), desc);
    }

    if (m_pBackBufferTexture != nullptr)
      m_pDevice->GetResourceManager()->ReleaseResource(m_pBackBufferTexture->GetHandle());

    m_pBackBufferTexture = spTextureD3D11::FromNative(ezStaticCast<spDeviceD3D11*>(m_pDevice), *pBackBufferTexture, spTextureDimension::Texture2D, spFromD3D11(m_eColorFormat));

    const spFramebufferDescription desc(m_pDepthTexture->GetHandle(), m_pBackBufferTexture->GetHandle());
    m_pFramebuffer = new spFramebufferD3D11(ezStaticCast<spDeviceD3D11*>(m_pDevice), desc);

    m_pDevice->GetResourceManager()->RegisterResource(m_pFramebuffer);
  }
}

spSwapchainD3D11::spSwapchainD3D11(spDeviceD3D11* pDevice, const spSwapchainDescription& description)
  : spSwapchain(description)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();

  m_eColorFormat = description.m_bUseSrgb ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;

  if (const auto* pRenderingSurfaceWin32 = ezDynamicCast<spRenderingSurfaceWin32*>(description.m_pRenderingSurface); pRenderingSurfaceWin32 != nullptr)
  {
    DXGI_SWAP_CHAIN_DESC desc;
    desc.BufferCount = 2;
    desc.Windowed = pRenderingSurfaceWin32->IsFullscreen() ? TRUE : FALSE;
    desc.BufferDesc.Width = description.m_uiWidth;
    desc.BufferDesc.Height = description.m_uiHeight;
    desc.BufferDesc.Format = m_eColorFormat;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    {
      spD3D11ScopedResource<IDXGIFactory> pDXGIFactory;
      pDevice->GetDXGIAdapter()->GetParent(IID_IDXGIFactory, reinterpret_cast<void**>(&pDXGIFactory));

      const HRESULT res = pDXGIFactory->CreateSwapChain(m_pD3D11Device, &desc, &m_pD3D11SwapChain);
      EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 swap chain resource. Error code: {}", (ezUInt32)HRESULT_CODE(res));

      pDXGIFactory->MakeWindowAssociation(pRenderingSurfaceWin32->GetMainWindowHandle(), DXGI_MWA_NO_ALT_ENTER);
    }
  }
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
  // TODO: Not sure if this one works properly, need to test...
  else if (const auto* pRenderingSurfaceUWP = ezDynamicCast<spRenderingSurfaceUWP*>(description.m_pRenderingSurface); pRenderingSurfaceUWP != nullptr)
  {
    m_fPixelScale = pRenderingSurfaceUWP->GetLogicalDpi() / 96.0f;

    DXGI_SWAP_CHAIN_DESC1 desc;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.BufferCount = 2;
    desc.Width = static_cast<UINT>(description.m_uiWidth * m_fPixelScale);
    desc.Height = static_cast<UINT>(description.m_uiHeight * m_fPixelScale);
    desc.Format = m_eColorFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    {
      spD3D11ScopedResource<IDXGIFactory2> pDXGIFactory;
      pDevice->GetDXGIAdapter()->GetParent(IID_IDXGIFactory2, reinterpret_cast<void**>(&pDXGIFactory));

      {
        spD3D11ScopedResource<IDXGISwapChain1> pDXGISwapChain1;
        pDXGIFactory->CreateSwapChainForComposition(m_pD3D11Device, &desc, nullptr, &pDXGISwapChain1);

        pDXGISwapChain1->QueryInterface(IID_IDXGISwapChain2, reinterpret_cast<void**>(&m_pD3D11SwapChain));
      }
    }

    auto* panelInspectable = static_cast<IInspectable*>(pRenderingSurfaceUWP->GetSwapchainPanelNative());

    ComPtr<ISwapChainPanelNative> pSwapChainNative;
    panelInspectable->QueryInterface(__uuidof(ISwapChainPanelNative), &pSwapChainNative);

    if (pSwapChainNative.Get() != nullptr)
    {
      pSwapChainNative->SetSwapChain(m_pD3D11SwapChain);
    }
    else
    {
      ComPtr<ISwapChainBackgroundPanelNative> pSwapChainBackgroundNative;
      panelInspectable->QueryInterface(__uuidof(ISwapChainBackgroundPanelNative), &pSwapChainBackgroundNative);

      if (pSwapChainBackgroundNative.Get() != nullptr)
        pSwapChainBackgroundNative->SetSwapChain(m_pD3D11SwapChain);
    }
  }
#endif

  Resize(description.m_uiWidth, description.m_uiHeight);
}

spSwapchainD3D11::~spSwapchainD3D11()
{
  spSwapchainD3D11::ReleaseResource();
}

void spSwapchainD3D11::AddCommandListReference(spCommandListD3D11* pCL)
{
  EZ_LOCK(m_CLsLock);
  m_DependentCommandLists.PushBack(pCL);
}

void spSwapchainD3D11::RemoveCommandListReference(spCommandListD3D11* pCL)
{
  EZ_LOCK(m_CLsLock);

  for (auto it = m_DependentCommandLists.GetIterator(); it.IsValid(); it.Next())
  {
    if (*it == pCL)
    {
      m_DependentCommandLists.Remove(it);
      break;
    }
  }
}
