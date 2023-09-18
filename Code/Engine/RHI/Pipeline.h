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

    /// \brief An array of \a spShaderSpecializationConstant used to override specialization constant values
    /// in the pipeline.
    ezDynamicArray<spShaderSpecializationConstant> m_Specializations{};
  };

  /// \brief Describes a \a spGraphicPipeline, for creation using a \a spDeviceResourceFactory.
  struct spGraphicPipelineDescription : ezHashableStruct<spGraphicPipelineDescription>
  {
    spRenderingState m_RenderingState;
    ezEnum<spPrimitiveTopology> m_ePrimitiveTopology;
    spShaderPipeline m_ShaderPipeline;
    spOutputDescription m_Output;
    ezDynamicArray<spResourceHandle> m_ResourceLayouts;

    /// \brief An array of \a spShaderSpecializationConstant used to override specialization constant values
    /// in the pipeline.
    ezDynamicArray<spShaderSpecializationConstant> m_Specializations{};
  };

  class SP_RHI_DLL spPipeline : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spPipeline, spDeviceResource);

  public:
    /// \brief Checks whether the pipeline is active.
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsActivated() const { return m_bActivated; }

    /// \brief Checks whether the pipeline is a compute pipeline.
    EZ_NODISCARD EZ_ALWAYS_INLINE virtual bool IsComputePipeline() const = 0;

    /// \brief Gets the shader program associated with this pipeline.
    EZ_NODISCARD EZ_ALWAYS_INLINE const spResourceHandle& GetShaderProgram() const { return m_hShaderProgram; }

    /// \brief Gets the resource layouts for this pipeline.
    EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<ezSharedPtr<spResourceLayout>>& GetResourceLayouts() const { return m_ResourceLayouts; }

    /// \brief Gets the resource layout at the given slot.
    /// \param uiSlot The slot to get the resource layout from.
    EZ_NODISCARD ezSharedPtr<spResourceLayout> GetResourceLayout(ezUInt32 uiSlot) const;

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
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsComputePipeline() const override { return true; }

  protected:
    explicit spComputePipeline(spComputePipelineDescription description);

    spComputePipelineDescription m_Description;
  };

  /// \brief A graphic pipeline.
  class SP_RHI_DLL spGraphicPipeline : public spPipeline
  {
    EZ_ADD_DYNAMIC_REFLECTION(spGraphicPipeline, spPipeline);

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsComputePipeline() const override { return false; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const spRenderingState& GetRenderingState() const { return m_Description.m_RenderingState; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezEnum<spPrimitiveTopology> GetPrimitiveTopology() const { return m_Description.m_ePrimitiveTopology; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const spOutputDescription& GetOutputDescription() const { return m_Description.m_Output; }

  protected:
    explicit spGraphicPipeline(spGraphicPipelineDescription description);

    spGraphicPipelineDescription m_Description;
  };
} // namespace RHI
