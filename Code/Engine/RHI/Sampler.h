#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Resource.h>

/// \brief Describes a sampler resource, for creation with a \see spDeviceResourceFactory.
struct SP_RHI_DLL spSamplerDescription : ezHashableStruct<spSamplerDescription>
{
  /// \brief Describes a common point filter sampler, with wrapping address mode.
  ///
  /// \code
  /// m_eAddressModeS = spSamplerAddressMode::ClampToEdge;
  /// m_eAddressModeT = spSamplerAddressMode::ClampToEdge;
  /// m_eAddressModeR = spSamplerAddressMode::ClampToEdge;
  /// m_eMinFilter = spSamplerFilter::Point;
  /// m_eMagFilter = spSamplerFilter::Point;
  /// m_iLodBias = 0;
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
  /// m_iLodBias = 0;
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
  /// m_iLodBias = 0;
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
  ezInt32 m_iLodBias{0};

  /// \brief The wrapping mode over the X axis.
  ezEnum<spSamplerAddressMode> m_eAddressModeS{spSamplerAddressMode::Default};

  /// \brief The wrapping mode over the Y axis.
  ezEnum<spSamplerAddressMode> m_eAddressModeT{spSamplerAddressMode::Default};

  /// \brief The wrapping mode over the Z axis.
  ezEnum<spSamplerAddressMode> m_eAddressModeR{spSamplerAddressMode::Default};

  /// \brief The border color when the wrapping mode is set to \see spSamplerWrapMode::BorderColor
  ezColor m_BorderColor{ezColor::Black};

  /// \brief The maximum anisotropy.
  ezUInt8 m_uiMaxAnisotropy{0};

  /// \brief The minification filtering mode.
  ezEnum<spSamplerFilter> m_eMinFilter{spSamplerFilter::Default};

  /// \brief The magnification filtering mode.
  ezEnum<spSamplerFilter> m_eMagFilter{spSamplerFilter::Default};

  /// \brief The sampling comparison.
  ezEnum<spDepthStencilComparison> m_eSamplerComparison{spDepthStencilComparison::Default};
};

class SP_RHI_DLL spSamplerState : public spDeviceResource
{
public:
  EZ_NODISCARD virtual spSamplerDescription GetSamplerDescription() const = 0;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spSamplerState);

class SP_RHI_DLL spSampler : public spShaderResource
{
public:
  EZ_NODISCARD virtual spSamplerState GetSamplerWithMipMap() const = 0;

  EZ_NODISCARD virtual spSamplerState GetSamplerWithoutMipMap() const = 0;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spSampler);
