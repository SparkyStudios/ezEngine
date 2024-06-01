#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>

namespace RHI
{
#pragma region Blend State

  /// \brief A \a spGraphicPipeline component describing the blend behavior of an individual color attachment.
  struct spBlendAttachment : ezHashableStruct<spBlendAttachment>
  {
    EZ_DECLARE_POD_TYPE();

    /// \brief Describes a \a spBlendAttachment state in which the source completely overrides the destination.
    const static spBlendAttachment OverrideBlend;

    /// \brief Describes a \a spBlendAttachment state in which the source and destination are blended in an inverse relationship.
    const static spBlendAttachment AlphaBlend;

    /// \brief Describes a \a spBlendAttachment state in which the source is added to the destination based on its alpha channel.
    const static spBlendAttachment AdditiveBlend;

    /// \brief Describes a \a spBlendAttachment state in which the source is multiplied with the destination.
    const static spBlendAttachment MultiplyBlend;

    /// \brief Describes a \a spBlendAttachment state in which blending is disabled.
    const static spBlendAttachment Disabled;

    /// \brief Compares this \a spBlendAttachment state to an \a other instance for equality.
    EZ_ALWAYS_INLINE bool operator==(const spBlendAttachment& other) const
    {
      return m_bEnabled == other.m_bEnabled && m_eSourceColorBlendFactor == other.m_eSourceColorBlendFactor && m_eDestinationColorBlendFactor == other.m_eDestinationColorBlendFactor && m_eColorBlendFunction == other.m_eColorBlendFunction && m_eSourceAlphaBlendFactor == other.m_eSourceAlphaBlendFactor && m_eDestinationAlphaBlendFactor == other.m_eDestinationAlphaBlendFactor && m_eAlphaBlendFunction == other.m_eAlphaBlendFunction;
    }

    /// \brief Compares this \a spBlendAttachment state to an \a other instance for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spBlendAttachment& other) const
    {
      return !(*this == other);
    }

    /// \brief Controls whether blending is enabled for the color attachment.
    bool m_bEnabled{false};

    /// \brief Controls which components of the color are used for blending.
    ezBitflags<spColorWriteMask> m_eColorWriteMask{spColorWriteMask::All};

    /// \brief Controls the source color's influence on the blended result.
    ezEnum<spBlendFactor> m_eSourceColorBlendFactor;

    /// \brief Controls the destination color's influence on the blended result.
    ezEnum<spBlendFactor> m_eDestinationColorBlendFactor;

    /// \brief Controls the function used to combine the source and the destination color blend factors.
    ezEnum<spBlendFunction> m_eColorBlendFunction;

    /// \brief Controls the source alpha's influence on the blended result.
    ezEnum<spBlendFactor> m_eSourceAlphaBlendFactor;

    /// \brief Controls the destination alpha's influence on the blended result.
    ezEnum<spBlendFactor> m_eDestinationAlphaBlendFactor;

    /// \brief Controls the function used to combine the source and the destination alpha blend factors.
    ezEnum<spBlendFunction> m_eAlphaBlendFunction;
  };

  /// \brief Describes the blend state of each color attachment in the \a spRenderingState.
  struct spBlendState : public ezHashableStruct<spBlendState>
  {
    /// \brief Describes an empty \a spBlendState.
    const static spBlendState Empty;

    /// \brief Describes a \a spBlendState in which a single color target is blended with \a spBlendAttachment::OverrideBlend.
    const static spBlendState SingleOverrideBlend;

    /// \brief Describes a \a spBlendState in which a single color target is blended with \a spBlendAttachment::AlphaBlend.
    const static spBlendState SingleAlphaBlend;

    /// \brief Describes a \a spBlendState in which a single color target is blended with \a spBlendAttachment::AdditiveBlend.
    const static spBlendState SingleAdditiveBlend;

    /// \brief Describes a \a spBlendState in which a single color target is blended with \a spBlendAttachment::MultiplyBlend.
    const static spBlendState SingleMultiplyBlend;

    /// \brief Describes a \a spBlendState in which a single color target is blended with \a spBlendAttachment::Disabled.
    const static spBlendState SingleDisabled;

    /// \brief The blend color.
    ezColor m_BlendColor;

    /// \brief An array of \a spBlendAttachment for each color attachment in the \a spGraphicPipeline.
    ezStaticArray<spBlendAttachment, SP_RHI_MAX_COLOR_TARGETS> m_AttachmentStates;

    /// \brief Enables alpha to coverage, which causes a fragment's alpha value to be used when determining multi-sample coverage.
    bool m_bAlphaToCoverage{false};
  };

#pragma endregion

#pragma region Depth State

  /// \brief Describes the depth state of a depth attachment in the \a spGraphicPipeline.
  struct spDepthState : ezHashableStruct<spDepthState>
  {
    EZ_DECLARE_POD_TYPE();

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::Less comparison.
    /// The stencil test is disabled.
    const static spDepthState Less;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::LessEqual comparison.
    /// The stencil test is disabled.
    const static spDepthState LessEqual;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::Less comparison.
    /// The stencil test is disabled.
    const static spDepthState LessRead;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::LessEqual comparison.
    /// The stencil test is disabled.
    const static spDepthState LessEqualRead;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::Greater comparison.
    /// The stencil test is disabled.
    const static spDepthState Greater;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::GreaterEqual comparison.
    /// The stencil test is disabled.
    const static spDepthState GreaterEqual;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::GreaterRead comparison.
    /// The stencil test is disabled.
    const static spDepthState GreaterRead;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::GreaterEqualRead comparison.
    /// The stencil test is disabled.
    const static spDepthState GreaterEqualRead;

    /// \brief Describes a \a spDepthState which uses a \a spDepthStencilComparison::Disabled comparison.
    /// The stencil test is disabled.
    const static spDepthState Disabled;

    /// \brief Defines whether the renderer has depth test capabilities enabled.
    bool m_bDepthTestEnabled{false};

    /// \brief The function used for depth comparison.
    ezEnum<spDepthStencilComparison> m_eDepthStencilComparison;

    /// \brief Defines whether the renderer can write to the depth mask.
    bool m_bDepthMaskEnabled{false};
  };

#pragma endregion

#pragma region Stencil State

  /// \brief Describes how stencil tests are performed in a \a spGraphicPipeline.
  struct spStencilBehavior : ezHashableStruct<spStencilBehavior>
  {
    EZ_DECLARE_POD_TYPE();

    spStencilBehavior() = default;

    spStencilBehavior(const ezEnum<spStencilOperation>& eFail, const ezEnum<spStencilOperation>& ePass, const ezEnum<spStencilOperation>& eDepthFail, const ezEnum<spDepthStencilComparison>& eComparison)
      : ezHashableStruct()
      , m_eFail(eFail)
      , m_ePass(ePass)
      , m_eDepthFail(eDepthFail)
      , m_eComparison(eComparison)
    {
    }

    /// \brief Compares this \a spStencilBehavior to an \a other instance for equality.
    EZ_ALWAYS_INLINE bool operator==(const spStencilBehavior& other) const
    {
      return m_eFail == other.m_eFail && m_ePass == other.m_ePass && m_eDepthFail == other.m_eDepthFail && m_eComparison == other.m_eComparison;
    }

    /// \brief Compares this \a spStencilBehavior to an \a other instance for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spStencilBehavior& other) const { return !(*this == other); }

    /// \brief The operation performed on samples that fails the stencil test.
    ezEnum<spStencilOperation> m_eFail;

    /// \brief The operation performed on samples that pass the stencil test.
    ezEnum<spStencilOperation> m_ePass;

    /// \brief The operation performed on samples that pass the stencil test but failed the depth test.
    ezEnum<spStencilOperation> m_eDepthFail;

    /// \brief The comparison operation used for stencil testing.
    ezEnum<spDepthStencilComparison> m_eComparison;
  };

  /// \brief Describes the stencil state of a depth attachment in the \a spGraphicPipeline.
  struct spStencilState : ezHashableStruct<spStencilState>
  {
    EZ_DECLARE_POD_TYPE();

    /// \brief Describes a disabled \a spStencilState.
    const static spStencilState Disabled;

    /// \brief Defines whether stencil testing is enabled.
    bool m_bEnabled{false};

    /// \brief The stencil test behavior for the front buffer.
    spStencilBehavior m_Front;

    /// \brief The stencil test behavior for the back buffer.
    spStencilBehavior m_Back;

    /// \brief Controls the portion of the stencil buffer used for reading.
    ezUInt8 m_uiReadMask{0};

    /// \brief Controls the portion of the stencil buffer used for writing.
    ezUInt8 m_uiWriteMask{0};

    /// \brief The reference value to use when doing a stencil test.
    ezUInt32 m_uiReference{0};
  };

#pragma endregion

#pragma region Rasterizer State

  /// \brief Describes how rasterization is performed in a \a spGraphicPipeline.
  struct spRasterizerState : ezHashableStruct<spRasterizerState>
  {
    EZ_DECLARE_POD_TYPE();

    /// \brief Describes the default rasterizer state, with clockwise backface culling, solid polygon filling
    /// and both depth clipping and scissor stencil tests enabled.
    const static spRasterizerState Default;

    /// \brief Describes a rasterizer state with no culling enabled, solid polygon filling, and both depth clipping
    /// and scissor stencil tests enabled.
    const static spRasterizerState CullNone;

    /// \brief Defines how front and back faces are culled.
    ezEnum<spFaceCullMode> m_eFaceCulling;

    /// \brief Defines whether the scissor test is enabled.
    bool m_bScissorTestEnabled{false};

    /// \brief Defines whether the depth clipping is enabled.
    bool m_bDepthClipEnabled{false};

    /// \brief Defines how the rasterizer will identify front faces from back faces.
    ezEnum<spFrontFace> m_eFrontFace;

    /// \brief Defines how the rasterizer will fill polygons.
    ezEnum<spPolygonFillMode> m_ePolygonFillMode;

    /// \brief Defines whether to enable conservative rasterization.
    bool m_bConservativeRasterization{false};

    /// \brief The pixel depth bias.
    float m_fDepthBias{0};

    /// \brief The pixel depth bias clamp.
    float m_fDepthBiasClamp{0.0f};

    /// \brief The pixel slope scaled depth bias.
    float m_fSlopeScaledDepthBias{0.0f};
  };

#pragma endregion

#pragma region Rendering State

  struct spRenderingState : ezHashableStruct<spRenderingState>
  {
    /// \brief The \a spBlendState of the rendering state.
    spBlendState m_BlendState{spBlendState::SingleDisabled};

    /// \brief The \a spDepthState of the rendering state.
    spDepthState m_DepthState{spDepthState::Disabled};

    /// \brief The \a spStencilState of the rendering state.
    spStencilState m_StencilState{spStencilState::Disabled};

    /// \brief The \a spRasterizerState of the rendering state.
    spRasterizerState m_RasterizerState{spRasterizerState::Default};
  };

#pragma endregion
} // namespace RHI

#include <RHI/Implementation/Rendering_inl.h>
