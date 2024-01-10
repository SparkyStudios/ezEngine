#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Resource.h>

namespace RHI
{
  /// \brief Describes a sampler resource, for creation with a \a spDeviceResourceFactory.
  struct spSamplerDescription : ezHashableStruct<spSamplerDescription>
  {
    /// \brief Describes a common point filter sampler, with wrapping address mode.
    ///
    /// \code
    /// m_eAddressModeS = spSamplerAddressMode::ClampToEdge;
    /// m_eAddressModeT = spSamplerAddressMode::ClampToEdge;
    /// m_eAddressModeR = spSamplerAddressMode::ClampToEdge;
    /// m_eMinFilter = spSamplerFilter::Point;
    /// m_eMagFilter = spSamplerFilter::Point;
    /// m_eMipFilter = spSamplerFilter::Point;
    /// m_fLodBias = 0;
    /// m_uiMinLod = 0;
    /// m_uiMaxLod = 0xffffffff;
    /// m_uiMaxAnisotropy = 0;
    /// \endcode
    static const spSamplerDescription Point;

    /// \brief Describes a common linear filter sampler, with wrapping address mode.
    ///
    /// \code
    /// m_eAddressModeS = spSamplerAddressMode::ClampToEdge;
    /// m_eAddressModeT = spSamplerAddressMode::ClampToEdge;
    /// m_eAddressModeR = spSamplerAddressMode::ClampToEdge;
    /// m_eMinFilter = spSamplerFilter::Linear;
    /// m_eMagFilter = spSamplerFilter::Linear;
    /// m_eMipFilter = spSamplerFilter::Linear;
    /// m_fLodBias = 0;
    /// m_uiMinLod = 0;
    /// m_uiMaxLod = 0xffffffff;
    /// m_uiMaxAnisotropy = 0;
    /// \endcode
    static const spSamplerDescription Linear;

    /// \brief Describes a common 4x anisotropic filter sampler, with wrapping address mode.
    ///
    /// \code
    /// m_eAddressModeS = spSamplerAddressMode::ClampToEdge;
    /// m_eAddressModeT = spSamplerAddressMode::ClampToEdge;
    /// m_eAddressModeR = spSamplerAddressMode::ClampToEdge;
    /// m_eMinFilter = spSamplerFilter::Linear;
    /// m_eMagFilter = spSamplerFilter::Linear;
    /// m_eMipFilter = spSamplerFilter::Linear;
    /// m_fLodBias = 0;
    /// m_uiMinLod = 0;
    /// m_uiMaxLod = 0xffffffff;
    /// m_uiMaxAnisotropy = 4;
    /// \endcode
    static const spSamplerDescription Anisotropic4x;

    /// \brief The minimum level of details.
    ezUInt32 m_uiMinLod{0};

    /// \brief The maximum level of details.
    ezUInt32 m_uiMaxLod{0xffffffff};

    /// \brief The level of details bias.
    float m_fLodBias{0.0f};

    /// \brief The wrapping mode over the X axis.
    ezEnum<spSamplerAddressMode> m_eAddressModeS{spSamplerAddressMode::Default};

    /// \brief The wrapping mode over the Y axis.
    ezEnum<spSamplerAddressMode> m_eAddressModeT{spSamplerAddressMode::Default};

    /// \brief The wrapping mode over the Z axis.
    ezEnum<spSamplerAddressMode> m_eAddressModeR{spSamplerAddressMode::Default};

    /// \brief The border color when the wrapping mode is set to \a spSamplerWrapMode::BorderColor
    ezEnum<spSamplerBorderColor> m_BorderColor{spSamplerBorderColor::Default};

    /// \brief The maximum anisotropy.
    ezUInt8 m_uiMaxAnisotropy{0};

    /// \brief The minification filtering mode.
    ezEnum<spSamplerFilter> m_eMinFilter{spSamplerFilter::Default};

    /// \brief The magnification filtering mode.
    ezEnum<spSamplerFilter> m_eMagFilter{spSamplerFilter::Default};

    /// \brief The mipmap filtering mode.
    ezEnum<spSamplerFilter> m_eMipFilter{spSamplerFilter::Default};

    /// \brief The sampling comparison.
    ezEnum<spDepthStencilComparison> m_eSamplerComparison{spDepthStencilComparison::Default};
  };

  class SP_RHI_DLL spSamplerState : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spSamplerState, spDeviceResource);

  public:
    EZ_NODISCARD virtual spSamplerDescription GetSamplerDescription() const = 0;
  };

  class SP_RHI_DLL spSampler : public spShaderResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spSampler, spShaderResource);

  public:
    EZ_NODISCARD virtual ezSharedPtr<spSamplerState> GetSamplerWithMipMap() const = 0;

    EZ_NODISCARD virtual ezSharedPtr<spSamplerState> GetSamplerWithoutMipMap() const = 0;
  };
} // namespace RHI

#include <RHI/Implementation/Sampler_inl.h>
