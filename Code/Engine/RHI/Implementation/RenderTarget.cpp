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
    m_pTexture.Clear();

    m_bReleased = true;
  }

  spRenderTarget::spRenderTarget(spDevice* pDevice, ezSharedPtr<spFramebuffer> pFramebuffer)
    : spDeviceResource()
  {
    m_pDevice = pDevice;

    m_pFramebuffer = pFramebuffer;
    m_pTexture = pDevice->GetResourceManager()->GetResource<spTexture>(pFramebuffer->GetColorTargets()[0]);

    m_bReleased = false;
  }

  spRenderTarget::spRenderTarget(spDevice* pDevice, ezSharedPtr<spTexture> pTexture)
    : spDeviceResource()
  {
    m_pDevice = pDevice;

    m_pTexture = pTexture;

    GenerateFramebuffer();

    m_bReleased = false;
  }

  spRenderTarget::spRenderTarget(spDevice* pDevice, const spRenderTargetDescription& description)
    : spDeviceResource()
    , m_Description(description)
  {
    m_pDevice = pDevice;

    ezBitflags<spTextureUsage> eTextureUsage = spTextureUsage::RenderTarget | spTextureUsage::Sampled;
    if (description.m_bGenerateMipMaps)
      eTextureUsage |= spTextureUsage::GenerateMipmaps;

    ezEnum<spPixelFormat> ePixelFormat = spPixelFormat::R8G8B8A8UNorm;
    if (description.m_eQuality == spRenderTargetQuality::HDR)
      ePixelFormat = spPixelFormat::R16G16B16A16Float;

    const spTextureDescription desc = spTextureDescription::Texture2D(description.m_uiWidth, description.m_uiHeight, description.m_bGenerateMipMaps ? description.m_uiMipmapsCount : 1, 1, ePixelFormat, eTextureUsage, description.m_eSampleCount);
    m_pTexture = m_pDevice->GetResourceFactory()->CreateTexture(desc);

    GenerateFramebuffer();

    m_bReleased = false;
  }

  void spRenderTarget::GenerateFramebuffer()
  {
    const spFramebufferDescription desc({}, m_pTexture->GetHandle());
    m_pFramebuffer = m_pDevice->GetResourceFactory()->CreateFramebuffer(desc);
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_RenderTarget);
