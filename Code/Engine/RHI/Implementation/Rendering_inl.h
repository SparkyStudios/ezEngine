#pragma once

namespace RHI
{
#pragma region spBlendAttachment

  inline const spBlendAttachment spBlendAttachment::OverrideBlend = {
    {},
    true,
    spColorWriteMask::All,
    spBlendFactor::One,
    spBlendFactor::Zero,
    spBlendFunction::Add,
    spBlendFactor::One,
    spBlendFactor::Zero,
    spBlendFunction::Add,
  };

  inline const spBlendAttachment spBlendAttachment::AlphaBlend = {
    {},
    true,
    spColorWriteMask::All,
    spBlendFactor::SourceAlpha,
    spBlendFactor::InverseSourceAlpha,
    spBlendFunction::Add,
    spBlendFactor::SourceAlpha,
    spBlendFactor::InverseSourceAlpha,
    spBlendFunction::Add,
  };

  inline const spBlendAttachment spBlendAttachment::AdditiveBlend = {
    {},
    true,
    spColorWriteMask::All,
    spBlendFactor::SourceAlpha,
    spBlendFactor::One,
    spBlendFunction::Add,
    spBlendFactor::SourceAlpha,
    spBlendFactor::One,
    spBlendFunction::Add,
  };

  inline const spBlendAttachment spBlendAttachment::MultiplyBlend = {
    {},
    true,
    spColorWriteMask::All,
    spBlendFactor::DestinationColor,
    spBlendFactor::InverseSourceAlpha,
    spBlendFunction::Add,
    spBlendFactor::DestinationAlpha,
    spBlendFactor::InverseSourceAlpha,
    spBlendFunction::Add,
  };

  inline const spBlendAttachment spBlendAttachment::Disabled = {
    {},
    false,
    spColorWriteMask::All,
    spBlendFactor::One,
    spBlendFactor::Zero,
    spBlendFunction::Add,
    spBlendFactor::One,
    spBlendFactor::Zero,
    spBlendFunction::Add,
  };

#pragma endregion

#pragma region spDepthState

  inline const spDepthState spDepthState::Less = {
    {},
    true,
    spDepthStencilComparison::Less,
    true,
  };

  inline const spDepthState spDepthState::LessEqual = {
    {},
    true,
    spDepthStencilComparison::LessEqual,
    true,
  };

  inline const spDepthState spDepthState::LessRead = {
    {},
    true,
    spDepthStencilComparison::Less,
    false,
  };

  inline const spDepthState spDepthState::LessEqualRead = {
    {},
    true,
    spDepthStencilComparison::LessEqual,
    false,
  };

  inline const spDepthState spDepthState::Greater = {
    {},
    true,
    spDepthStencilComparison::Greater,
    true,
  };

  inline const spDepthState spDepthState::GreaterEqual = {
    {},
    true,
    spDepthStencilComparison::GreaterEqual,
    true,
  };

  inline const spDepthState spDepthState::GreaterRead = {
    {},
    true,
    spDepthStencilComparison::Greater,
    false,
  };

  inline const spDepthState spDepthState::GreaterEqualRead = {
    {},
    true,
    spDepthStencilComparison::GreaterEqual,
    false,
  };

  inline const spDepthState spDepthState::Disabled = {
    {},
    false,
    spDepthStencilComparison::Always,
    false,
  };

#pragma endregion

#pragma region spBlendState

  inline const spBlendState spBlendState::SingleOverrideBlend = {
    {},
    ezColorGammaUB(0x00, 0x00, 0x00),
    ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::OverrideBlend, 1)),
  };

  inline const spBlendState spBlendState::SingleAlphaBlend = {
    {},
    ezColorGammaUB(0x00, 0x00, 0x00),
    ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::AlphaBlend, 1)),
  };

  inline const spBlendState spBlendState::SingleAdditiveBlend = {
    {},
    ezColorGammaUB(0x00, 0x00, 0x00),
    ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::AdditiveBlend, 1)),
  };

  inline const spBlendState spBlendState::SingleMultiplyBlend = {
    {},
    ezColorGammaUB(0x00, 0x00, 0x00),
    ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::MultiplyBlend, 1)),
  };

  inline const spBlendState spBlendState::SingleDisabled = {
    {},
    ezColorGammaUB(0x00, 0x00, 0x00),
    ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::Disabled, 1)),
  };

#pragma endregion

#pragma region spStencilState

  inline const spStencilState spStencilState::Disabled = {
    {},
    false,
  };

#pragma endregion

#pragma region spRasterizerState

  inline const spRasterizerState spRasterizerState::Default = {
    {},
    spFaceCullMode::Back,
    true,
    true,
    spFrontFace::Clockwise,
    spPolygonFillMode::Solid,
  };

  inline const spRasterizerState spRasterizerState::CullNone = {
    {},
    spFaceCullMode::None,
    true,
    true,
    spFrontFace::Clockwise,
    spPolygonFillMode::Solid,
  };

#pragma endregion
} // namespace RHI
