#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Output.h>
#include <RHI/Rendering.h>
#include <RHI/Resource.h>
#include <RHI/ResourceLayout.h>
#include <RHI/Shader.h>

namespace RHI
{
  /// \brief Describes a \a spComputePipeline, for creation using a \a spDeviceResourceFactory.
  struct spComputePipelineDescription : ezHashableStruct<spComputePipelineDescription>
  {
    spComputePipelineDescription() = default;

    spComputePipelineDescription(spResourceHandle hShader, ezDynamicArray<spResourceHandle> resourceLayouts, ezUInt32 uiThreadGroupSizeX, ezUInt32 uiThreadGroupSizeY, ezUInt32 uiThreadGroupSizeZ)
      : ezHashableStruct()
      , m_hComputeShader(hShader)
      , m_ResourceLayouts(std::move(resourceLayouts))
      , m_uiThreadGroupSizeX(uiThreadGroupSizeX)
      , m_uiThreadGroupSizeY(uiThreadGroupSizeY)
      , m_uiThreadGroupSizeZ(uiThreadGroupSizeZ)
    {
    }

    spComputePipelineDescription(spResourceHandle hShader, const spResourceLayout* resourceLayout, ezUInt32 uiThreadGroupSizeX, ezUInt32 uiThreadGroupSizeY, ezUInt32 uiThreadGroupSizeZ)
      : ezHashableStruct()
      , m_hComputeShader(hShader)
      , m_ResourceLayouts()
      , m_uiThreadGroupSizeX(uiThreadGroupSizeX)
      , m_uiThreadGroupSizeY(uiThreadGroupSizeY)
      , m_uiThreadGroupSizeZ(uiThreadGroupSizeZ)
    {
      m_ResourceLayouts.PushBack(resourceLayout->GetHandle());
    }

    /// \brief Compares the current instance with the \a other instance for equality.
    EZ_ALWAYS_INLINE bool operator==(const spComputePipelineDescription& other) const
    {
      return m_hComputeShader == other.m_hComputeShader && m_ResourceLayouts == other.m_ResourceLayouts && m_uiThreadGroupSizeX == other.m_uiThreadGroupSizeX && m_uiThreadGroupSizeY == other.m_uiThreadGroupSizeY && m_uiThreadGroupSizeZ == other.m_uiThreadGroupSizeZ;
    }

    /// \brief Compares the current instance with the \a other instance for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spComputePipelineDescription& other) const
    {
      return !(*this == other);
    }

    /// \brief An handle to the compute shader to use in this pipeline. The shader must be created
    /// with the appropriate stage (\a spShaderStage).
    spResourceHandle m_hComputeShader{};

    /// \brief An array of \a spResourceLayout, which controls the layout of the shader
    /// resources in this pipeline.
    ezDynamicArray<spResourceHandle> m_ResourceLayouts{};

    /// \brief The X dimension of the thread group size.
    ezUInt32 m_uiThreadGroupSizeX{1};

    /// \brief The Y dimension of the thread group size.
    ezUInt32 m_uiThreadGroupSizeY{1};

    /// \brief The Z dimension of the thread group size.
    ezUInt32 m_uiThreadGroupSizeZ{1};

    /// \brief Whether the pipeline supports push constants. This is only relevant for
    /// Metal and D3D11 backends where the resource binding model is different and there
    /// is no native support of push constants.
    bool m_bSupportsPushConstants{true};
  };

  /// \brief Describes a \a spGraphicPipeline, for creation using a \a spDeviceResourceFactory.
  struct spGraphicPipelineDescription : ezHashableStruct<spGraphicPipelineDescription>
  {
    /// \brief Defines the rendering state of the graphics pipeline.
    spRenderingState m_RenderingState;

    /// \brief The primitive topology to use.
    ezEnum<spPrimitiveTopology> m_ePrimitiveTopology;

    /// \brief The shader pipeline to use.
    spShaderPipeline m_ShaderPipeline;

    /// \brief The framebuffer's output description.
    spOutputDescription m_Output;

    /// \brief An array of \a spResourceLayout, which controls the layout of the shader
    /// resources in this pipeline.
    ezDynamicArray<spResourceHandle> m_ResourceLayouts;

    /// \brief Whether the pipeline supports push constants. This is only relevant for
    /// Metal and D3D11 backends where the resource binding model is different and there
    /// is no native support of push constants.
    bool m_bSupportsPushConstants{false};
  };

  class SP_RHI_DLL spPipeline : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spPipeline, spDeviceResource);

  public:
    /// \brief Checks whether the pipeline is active.
    [[nodiscard]] EZ_ALWAYS_INLINE bool IsActivated() const { return m_bActivated; }

    /// \brief Checks whether the pipeline is a compute pipeline.
    [[nodiscard]] EZ_ALWAYS_INLINE virtual bool IsComputePipeline() const = 0;

    /// \brief Gets the shader program associated with this pipeline.
    [[nodiscard]] EZ_ALWAYS_INLINE const spResourceHandle& GetShaderProgram() const { return m_hShaderProgram; }

    /// \brief Gets the resource layouts for this pipeline.
    [[nodiscard]] EZ_ALWAYS_INLINE const ezDynamicArray<ezSharedPtr<spResourceLayout>>& GetResourceLayouts() const { return m_ResourceLayouts; }

    /// \brief Gets the resource layout at the given slot.
    /// \param uiSlot The slot to get the resource layout from.
    [[nodiscard]] ezSharedPtr<spResourceLayout> GetResourceLayout(ezUInt32 uiSlot) const;

  protected:
    bool m_bActivated{false};
    spResourceHandle m_hShaderProgram;
    ezDynamicArray<ezSharedPtr<spResourceLayout>> m_ResourceLayouts;
  };

  /// \brief A compute pipeline.
  class SP_RHI_DLL spComputePipeline : public spPipeline
  {
    EZ_ADD_DYNAMIC_REFLECTION(spComputePipeline, spPipeline);

  public:
    [[nodiscard]] EZ_ALWAYS_INLINE bool IsComputePipeline() const override { return true; }

    [[nodiscard]] EZ_ALWAYS_INLINE bool SupportsPushConstants() const { return m_Description.m_bSupportsPushConstants; }

  protected:
    explicit spComputePipeline(spComputePipelineDescription description);

    spComputePipelineDescription m_Description;
  };

  /// \brief A graphic pipeline.
  class SP_RHI_DLL spGraphicPipeline : public spPipeline
  {
    EZ_ADD_DYNAMIC_REFLECTION(spGraphicPipeline, spPipeline);

  public:
    [[nodiscard]] EZ_ALWAYS_INLINE bool IsComputePipeline() const override { return false; }

    [[nodiscard]] EZ_ALWAYS_INLINE const spRenderingState& GetRenderingState() const { return m_Description.m_RenderingState; }

    [[nodiscard]] EZ_ALWAYS_INLINE ezEnum<spPrimitiveTopology> GetPrimitiveTopology() const { return m_Description.m_ePrimitiveTopology; }

    [[nodiscard]] EZ_ALWAYS_INLINE const spOutputDescription& GetOutputDescription() const { return m_Description.m_Output; }

    [[nodiscard]] EZ_ALWAYS_INLINE bool SupportsPushConstants() const { return m_Description.m_bSupportsPushConstants; }

    [[nodiscard]] EZ_ALWAYS_INLINE const spGraphicPipelineDescription& GetDescription() const { return m_Description; }

  protected:
    explicit spGraphicPipeline(spGraphicPipelineDescription description);

    spGraphicPipelineDescription m_Description;
  };
} // namespace RHI
