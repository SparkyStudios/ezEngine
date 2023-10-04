#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Framebuffer.h>
#include <RHID3D11/Texture.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFramebufferD3D11, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spFramebufferD3D11::SetDebugName(ezStringView sDebugName)
  {
    spFramebuffer::SetDebugName(sDebugName);

    ezStringBuilder sDSVName(sDebugName);
    sDSVName.Append("_DSV");

    ezStringBuilder sRTVName(sDebugName);
    sRTVName.Append("_RTV");

    if (m_pDepthTarget != nullptr)
      m_pDepthTarget->SetPrivateData(WKPDID_D3DDebugObjectName, sDSVName.GetElementCount(), sDSVName.GetData());

    for (auto& target : m_ColorTargets)
      target->SetPrivateData(WKPDID_D3DDebugObjectName, sRTVName.GetElementCount(), sRTVName.GetData());
  }

  void spFramebufferD3D11::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_DX11_RELEASE(m_pDepthTarget);

    for (auto& target : m_ColorTargets)
      SP_RHI_DX11_RELEASE(target);

    m_ColorTargets.Clear();

    m_bIsResourceCreated = false;
  }

  bool spFramebufferD3D11::IsReleased() const
  {
    return !m_bIsResourceCreated;
  }

  void spFramebufferD3D11::CreateResource()
  {
    const spTextureD3D11* pDimensionTexture = nullptr;
    ezUInt32 uiDimensionMipLevel = 0;

    if (!m_Description.m_DepthTarget.m_hTarget.IsInvalidated())
    {
      auto pDepthTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(m_Description.m_DepthTarget.m_hTarget);
      EZ_ASSERT_DEV(pDepthTexture != nullptr, "Unable to find a texture in the device resource manager. If you have created that resource yourself, make sure to register it in the manager.");
      pDepthTexture->EnsureResourceCreated();

      pDimensionTexture = pDepthTexture;
      uiDimensionMipLevel = m_Description.m_DepthTarget.m_uiMipLevel;

      D3D11_DEPTH_STENCIL_VIEW_DESC desc;
      desc.Format = spGetDepthFormat(pDepthTexture->GetFormat());
      desc.Flags = 0;

      if (pDepthTexture->GetArrayLayerCount() == 1)
      {
        if (pDepthTexture->GetSampleCount() == spTextureSampleCount::None)
        {
          desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
          desc.Texture2D.MipSlice = m_Description.m_DepthTarget.m_uiMipLevel;
        }
        else
        {
          desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
      }
      else
      {
        if (pDepthTexture->GetSampleCount() == spTextureSampleCount::None)
        {
          desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
          desc.Texture2DArray.MipSlice = m_Description.m_DepthTarget.m_uiMipLevel;
          desc.Texture2DArray.ArraySize = 1;
          desc.Texture2DArray.FirstArraySlice = m_Description.m_DepthTarget.m_uiArrayLayer;
        }
        else
        {
          desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
          desc.Texture2DArray.ArraySize = 1;
          desc.Texture2DArray.FirstArraySlice = m_Description.m_DepthTarget.m_uiArrayLayer;
        }
      }

      const HRESULT res = m_pD3D11Device->CreateDepthStencilView(pDepthTexture->GetD3D11Texture(), &desc, &m_pDepthTarget);
      EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 depth stencil view resource. Error Code: {}", (ezUInt32)HRESULT_CODE(res));
    }

    if (m_Description.m_ColorTargets.GetCount() > 0)
    {
      m_ColorTargets.EnsureCount(m_Description.m_ColorTargets.GetCount());
      for (ezUInt32 i = 0, l = m_Description.m_ColorTargets.GetCount(); i < l; ++i)
      {
        const auto& target = m_Description.m_ColorTargets[i];

        if (i == 0)
        {
          auto pColorTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(target.m_hTarget);
          EZ_ASSERT_DEV(pColorTexture != nullptr, "Unable to find a texture in the device resource manager. If you have created that texture yourself, make sure to register it in the manager.");
          pColorTexture->EnsureResourceCreated();

          pDimensionTexture = pColorTexture;
          uiDimensionMipLevel = target.m_uiMipLevel;
        }

        ApplyColorTarget(i, target);
      }

      if (pDimensionTexture != nullptr)
      {
        ezUInt32 uiDepth;
        spTextureHelper::GetMipDimensions(pDimensionTexture, uiDimensionMipLevel, m_uiWidth, m_uiHeight, uiDepth);
      }

      m_bIsResourceCreated = true;
    }
  }

  void spFramebufferD3D11::SetColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target)
  {
    EZ_ASSERT_DEV(m_ColorTargets.GetCount() > uiIndex, "Invalid color target index. Valid indexes are between 0 and {}.", m_ColorTargets.GetCount());

    EnsureResourceCreated();
    ApplyColorTarget(uiIndex, target);
  }

  void spFramebufferD3D11::Snapshot(ezUInt32 uiColorTargetIndex, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel, ezByteArrayPtr& out_Pixels)
  {
    EZ_ASSERT_DEV(m_ColorTargets.GetCount() > uiColorTargetIndex, "Invalid color target index. Valid indexes are between 0 and {}.", m_ColorTargets.GetCount());
    EZ_ASSERT_NOT_IMPLEMENTED
  }

  spFramebufferD3D11::spFramebufferD3D11(spDeviceD3D11* pDevice, const spFramebufferDescription& description)
    : spFramebuffer(description)
    , m_OutputDescription()
  {
    m_pDevice = pDevice;
    m_pD3D11Device = pDevice->GetD3D11Device();

    m_OutputDescription = spOutputDescription::CreateFromFramebuffer(this);
  }

  spFramebufferD3D11::~spFramebufferD3D11()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  ezSharedPtr<spTextureD3D11> spFramebufferD3D11::GetColorTarget(ezUInt32 uiColorTargetIndex) const
  {
    auto pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(m_Description.m_ColorTargets[uiColorTargetIndex].m_hTarget);
    EZ_ASSERT_DEV(pTexture != nullptr, "Invalid color target texture.");

    return pTexture;
  }

  void spFramebufferD3D11::ApplyColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target)
  {
    auto pColorTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(target.m_hTarget);
    EZ_ASSERT_DEV(pColorTexture != nullptr, "Unable to find a texture in the device resource manager. If you have created that texture yourself, make sure to register it in the manager.");
    pColorTexture->EnsureResourceCreated();

    D3D11_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = spToD3D11(pColorTexture->GetFormat(), false);

    if (pColorTexture->GetArrayLayerCount() > 1 || pColorTexture->GetUsage().IsSet(spTextureUsage::Cubemap))
    {
      if (pColorTexture->GetSampleCount() == spTextureSampleCount::None)
      {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.MipSlice = target.m_uiMipLevel;
        desc.Texture2DArray.ArraySize = 1;
        desc.Texture2DArray.FirstArraySlice = target.m_uiArrayLayer;
      }
      else
      {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
        desc.Texture2DArray.ArraySize = 1;
        desc.Texture2DArray.FirstArraySlice = target.m_uiArrayLayer;
      }
    }
    else
    {
      if (pColorTexture->GetSampleCount() == spTextureSampleCount::None)
      {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = target.m_uiMipLevel;
      }
      else
      {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
      }
    }

    // In case we are overriding an existing color target
    SP_RHI_DX11_RELEASE(m_ColorTargets[uiIndex]);

    const HRESULT res = m_pD3D11Device->CreateRenderTargetView(pColorTexture->GetD3D11Texture(), &desc, &m_ColorTargets[uiIndex]);
    EZ_ASSERT_DEV(SUCCEEDED(res), "Failed to create a D3D11 render target view resource. Error code: {}", (ezUInt32)HRESULT_CODE(res));
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Framebuffer);
