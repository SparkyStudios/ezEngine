#include <RHIMTL/Device.h>
#include <RHIMTL/Framebuffer.h>
#include <RHIMTL/Swapchain.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

namespace RHI
{
  CA::MetalLayer* spRenderingSurfaceNSWindow::GetMetalLayer() const
  {
    auto* pView = [(__bridge NSWindow*)m_pWindow contentView];
    CALayer* pLayer = [pView layer];

    if ([pLayer isKindOfClass:[CAMetalLayer class]])
      return (__bridge CA::MetalLayer*)pLayer;

    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    pView.layer = metalLayer;
    pView.wantsLayer = YES;

    return (__bridge CA::MetalLayer*)metalLayer;
  }

  ezUInt32 spRenderingSurfaceNSWindow::GetWidth() const
  {
    auto* pView = [(__bridge NSWindow*)m_pWindow contentView];
    return [pView frame].size.width;
  }

  ezUInt32 spRenderingSurfaceNSWindow::GetHeight() const
  {
    auto* pView = [(__bridge NSWindow*)m_pWindow contentView];
    return [pView frame].size.height;
  }

  void spSwapchainMTL::SetVSync(bool bVSync)
  {
    if (m_bVSync == bVSync)
      return;

    m_bVSync = bVSync;

    if (m_pMetalLayer == nullptr)
      return;

    auto* pDevice = static_cast<spDeviceMTL*>(m_pDevice);
    const auto& supportedFeatures = pDevice->GetSupportedFeatures();

    if (supportedFeatures.GetMaxFeatureSet() == MTL::FeatureSet_macOS_GPUFamily1_v3 || supportedFeatures.GetMaxFeatureSet() == MTL::FeatureSet_macOS_GPUFamily1_v4 || supportedFeatures.GetMaxFeatureSet() == MTL::FeatureSet_macOS_GPUFamily2_v1)
    {
      auto* pLayer = (__bridge CAMetalLayer*)m_pMetalLayer;
      pLayer.displaySyncEnabled = bVSync ? YES : NO;
    }
  }

  void spSwapchainMTL::Present()
  {
    if (m_pMetalDrawable != nullptr)
    {
      auto* pDevice = static_cast<spDeviceMTL*>(m_pDevice);

      @autoreleasepool
      {
        MTL::CommandBuffer* submitCB = pDevice->GetCommandQueue()->commandBuffer();
        submitCB->presentDrawable(m_pMetalDrawable);
        submitCB->commit();
      }
    }
  }

  void spSwapchainMTL::GetNextDrawable()
  {
    SP_RHI_MTL_RELEASE(m_pMetalDrawable);

    @autoreleasepool
    {
      m_pMetalDrawable = m_pMetalLayer->nextDrawable();
      SP_RHI_MTL_RETAIN(m_pMetalDrawable);

      m_pSwapchainFramebuffer->SetDrawableTexture(m_pMetalDrawable->texture());
    }
  }
} // namespace RHI
