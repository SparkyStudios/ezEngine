#include <RHI/RHIPCH.h>

#include <RHI/Rendering.h>

#pragma region spBlendAttachment

const spBlendAttachment spBlendAttachment::OverrideBlend = {
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

const spBlendAttachment spBlendAttachment::AlphaBlend = {
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

const spBlendAttachment spBlendAttachment::AdditiveBlend = {
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

const spBlendAttachment spBlendAttachment::MultiplyBlend = {
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

const spBlendAttachment spBlendAttachment::Disabled = {
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

#pragma region spBlendState

const spBlendState spBlendState::SingleOverrideBlend = {
  {},
  ezColor::Black,
  ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::OverrideBlend, 1)),
};

const spBlendState spBlendState::SingleAlphaBlend = {
  {},
  ezColor::Black,
  ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::AlphaBlend, 1)),
};

const spBlendState spBlendState::SingleAdditiveBlend = {
  {},
  ezColor::Black,
  ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::AdditiveBlend, 1)),
};

const spBlendState spBlendState::SingleMultiplyBlend = {
  {},
  ezColor::Black,
  ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::MultiplyBlend, 1)),
};

const spBlendState spBlendState::SingleDisabled = {
  {},
  ezColor::Black,
  ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS>(ezMakeArrayPtr(&spBlendAttachment::Disabled, 1)),
};

#pragma endregion

#pragma region spDepthState

const spDepthState spDepthState::Less = {
  {},
  true,
  spDepthStencilComparison::Less,
  true,
};

const spDepthState spDepthState::LessEqual = {
  {},
  true,
  spDepthStencilComparison::LessEqual,
  true,
};

const spDepthState spDepthState::LessRead = {
  {},
  true,
  spDepthStencilComparison::Less,
  false,
};

const spDepthState spDepthState::LessEqualRead = {
  {},
  true,
  spDepthStencilComparison::LessEqual,
  false,
};

const spDepthState spDepthState::Greater = {
  {},
  true,
  spDepthStencilComparison::Greater,
  true,
};

const spDepthState spDepthState::GreaterEqual = {
  {},
  true,
  spDepthStencilComparison::GreaterEqual,
  true,
};

const spDepthState spDepthState::GreaterRead = {
  {},
  true,
  spDepthStencilComparison::Greater,
  false,
};

const spDepthState spDepthState::GreaterEqualRead = {
  {},
  true,
  spDepthStencilComparison::GreaterEqual,
  false,
};

const spDepthState spDepthState::Disabled = {
  {},
  false,
  spDepthStencilComparison::Always,
  false,
};

#pragma endregion

#pragma region spStencilState

const spStencilState spStencilState::Disabled = {
  {},
  false,
};

#pragma endregion

#pragma region spRasterizerState

const spRasterizerState spRasterizerState::Default = {
  {},
  spFaceCullMode::Back,
  true,
  true,
  spFrontFace::Clockwise,
  spPolygonFillMode::Solid,
};

const spRasterizerState spRasterizerState::CullNone = {
  {},
  spFaceCullMode::None,
  true,
  true,
  spFrontFace::Clockwise,
  spPolygonFillMode::Solid,
};

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Rendering);
