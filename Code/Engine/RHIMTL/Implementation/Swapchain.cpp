#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Core.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Framebuffer.h>
#include <RHIMTL/Swapchain.h>
#include <RHIMTL/Texture.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderingSurfaceNSWindow, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSwapchainMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spSwapchainMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_MTL_RELEASE(m_pMetalDrawable);

    m_pSwapchainFramebuffer.Clear();
    SP_RHI_MTL_RELEASE(m_pMetalLayer);

    m_pMetalDrawable = nullptr;
    m_pMetalLayer = nullptr;
  }

  bool spSwapchainMTL::IsReleased() const
  {
    return m_pMetalLayer == nullptr && m_pMetalDrawable == nullptr;
  }

  void spSwapchainMTL::SetDebugName(ezStringView sDebugName)
  {
    spSwapchain::SetDebugName(sDebugName);

    if (IsReleased() || sDebugName.IsEmpty())
      return;

    // TODO: Set the debug name of the Metal layer
  }

  ezSharedPtr<spFramebuffer> spSwapchainMTL::GetFramebuffer() const
  {
    return m_pSwapchainFramebuffer;
  }

  void spSwapchainMTL::Resize(ezUInt32 uiWidth, ezUInt32 uiHeight)
  {
    m_pSwapchainFramebuffer->Resize(uiWidth, uiHeight);
    m_pMetalLayer->setDrawableSize(CGSizeMake(uiWidth, uiHeight));
    GetNextDrawable();
  }

  spSwapchainMTL::spSwapchainMTL(spDeviceMTL* pDevice, const spSwapchainDescription& description)
    : spSwapchain(description)
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();

    ezUInt32 uiWidth = description.m_uiWidth;
    ezUInt32 uiHeight = description.m_uiHeight;

    if (auto* pSurface = ezDynamicCast<spRenderingSurfaceNSWindow*>(description.m_pRenderingSurface); pSurface != nullptr)
    {
      uiWidth = pSurface->GetWidth();
      uiHeight = pSurface->GetHeight();

      m_pMetalLayer = pSurface->GetMetalLayer();
    }
    else
    {
      EZ_ASSERT_DEV(false, "Metal Swapchain only supports NSView as rendering surface");
    }

    const ezEnum<spPixelFormat>& eFormat = description.m_bUseSrgb ? spPixelFormat::R8G8B8A8UNormSRgb : spPixelFormat::R8G8B8A8UNorm;

    m_pMetalLayer->setDevice(m_pMTLDevice);
    m_pMetalLayer->setPixelFormat(spToMTL(eFormat, false));
    m_pMetalLayer->setFramebufferOnly(false);
    m_pMetalLayer->setDrawableSize(CGSizeMake(uiWidth, uiHeight));

    m_pSwapchainFramebuffer = EZ_NEW(m_pDevice->GetAllocator(), spSwapchainFramebufferMTL, pDevice, this, uiWidth, uiHeight, eFormat, description.m_eDepthFormat);
    pDevice->GetResourceManager()->RegisterResource(m_pSwapchainFramebuffer);

    SetVSync(description.m_bVSync);
    GetNextDrawable();
  }

  spSwapchainMTL::~spSwapchainMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Swapchain);
