#include <RHI/RHIPCH.h>

#include <RHI/RenderTarget.h>

#include <RHI/Device.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderTarget, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spRenderTarget::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_pFramebuffer.Clear();
    m_pColorTexture.Clear();

    m_bReleased = true;
  }

  spRenderTarget::spRenderTarget(spDevice* pDevice, ezSharedPtr<spFramebuffer> pFramebuffer)
    : spDeviceResource()
  {
    m_pDevice = pDevice;

    m_pFramebuffer = pFramebuffer;
    m_pColorTexture = pDevice->GetResourceManager()->GetResource<spTexture>(pFramebuffer->GetColorTargets()[0]);

    m_bReleased = false;
  }

  spRenderTarget::spRenderTarget(spDevice* pDevice, ezSharedPtr<spTexture> pTexture)
    : spDeviceResource()
  {
    m_pDevice = pDevice;

    m_pColorTexture = pTexture;

    GenerateFramebuffer();

    m_bReleased = false;
  }

  spRenderTarget::spRenderTarget(spDevice* pDevice, const spRenderTargetDescription& description)
    : spDeviceResource()
    , m_Description(description)
  {
    m_pDevice = pDevice;

    ezBitflags<spTextureUsage> eColorTextureUsage = spTextureUsage::RenderTarget | spTextureUsage::Sampled;
    ezBitflags<spTextureUsage> eDepthTextureUsage = spTextureUsage::DepthStencil | spTextureUsage::Sampled;
    if (description.m_bGenerateMipMaps)
    {
      eColorTextureUsage |= spTextureUsage::GenerateMipmaps;
      eDepthTextureUsage |= spTextureUsage::GenerateMipmaps;
    }

    ezEnum<spPixelFormat> ePixelFormat = spPixelFormat::R8G8B8A8UNorm;
    if (description.m_eQuality == spRenderTargetQuality::HDR)
      ePixelFormat = spPixelFormat::R16G16B16A16Float;

    const spTextureDescription colorDesc = spTextureDescription::Texture2D(description.m_uiWidth, description.m_uiHeight, description.m_bGenerateMipMaps ? description.m_uiMipmapsCount : 1, 1, ePixelFormat, eColorTextureUsage, description.m_eSampleCount);
    m_pColorTexture = m_pDevice->GetResourceFactory()->CreateTexture(colorDesc);

    if (description.m_bHasDepth)
    {
      const spTextureDescription depthDesc = spTextureDescription::Texture2D(description.m_uiWidth, description.m_uiHeight, description.m_bGenerateMipMaps ? description.m_uiMipmapsCount : 1, 1, description.m_bHasStencil ? spPixelFormat::D24UNormS8UInt : spPixelFormat::D32Float, eDepthTextureUsage, description.m_eSampleCount);
      m_pDepthStencilTexture = m_pDevice->GetResourceFactory()->CreateTexture(depthDesc);
    }

    GenerateFramebuffer();

    m_bReleased = false;
  }

  void spRenderTarget::GenerateFramebuffer()
  {
    const spFramebufferDescription desc(m_pDepthStencilTexture != nullptr ? m_pDepthStencilTexture->GetHandle() : spResourceHandle(), m_pColorTexture->GetHandle());
    m_pFramebuffer = m_pDevice->GetResourceFactory()->CreateFramebuffer(desc);
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_RenderTarget);
