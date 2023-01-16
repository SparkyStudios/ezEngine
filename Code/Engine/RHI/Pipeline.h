#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>
#include <RHI/ResourceLayout.h>
#include <RHI/Shader.h>

/// \brief Describes a \see spComputePipeline, for creation using a \see spDeviceResourceFactory.
struct SP_RHI_DLL spComputePipelineDescription : public ezHashableStruct<spComputePipelineDescription>
{
  spComputePipelineDescription()
    : ezHashableStruct<spComputePipelineDescription>()
  {
  }

  spComputePipelineDescription(spResourceHandle hShader, ezDynamicArray<spResourceHandle> resourceLayouts, ezUInt32 uiThreadGroupSizeX, ezUInt32 uiThreadGroupSizeY, ezUInt32 uiThreadGroupSizeZ)
    : ezHashableStruct<spComputePipelineDescription>()
    , m_hComputeShader(std::move(hShader))
    , m_ResourceLayouts(std::move(resourceLayouts))
    , m_uiThreadGroupSizeX(uiThreadGroupSizeX)
    , m_uiThreadGroupSizeY(uiThreadGroupSizeY)
    , m_uiThreadGroupSizeZ(uiThreadGroupSizeZ)
  {
  }

  spComputePipelineDescription(spResourceHandle hShader, const spResourceLayout* resourceLayout, ezUInt32 uiThreadGroupSizeX, ezUInt32 uiThreadGroupSizeY, ezUInt32 uiThreadGroupSizeZ)
    : ezHashableStruct<spComputePipelineDescription>()
    , m_hComputeShader(std::move(hShader))
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
  /// with the appropriate stage (\see spShaderStage).
  spResourceHandle m_hComputeShader{};

  /// \brief An array of \see spResourceLayout, which controls the layout of the shader
  /// resources in this pipeline.
  ezDynamicArray<spResourceHandle> m_ResourceLayouts{};

  /// \brief The X dimension of the thread group size.
  ezUInt32 m_uiThreadGroupSizeX{1};

  /// \brief The Y dimension of the thread group size.
  ezUInt32 m_uiThreadGroupSizeY{1};

  /// \brief The Z dimension of the thread group size.
  ezUInt32 m_uiThreadGroupSizeZ{1};

  /// \brief An array of \see spShaderSpecializationConstant used to override specialization constant values
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
  EZ_NODISCARD EZ_ALWAYS_INLINE const spResourceHandle& GetShaderProgram() const { return m_ShaderProgram; }

  /// \brief Gets the resource layouts for this pipeline.
  EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<spResourceHandle>& GetResourceLayouts() const { return m_ResourceLayouts; }

protected:
  bool m_bActivated;
  spResourceHandle m_ShaderProgram;
  ezDynamicArray<spResourceHandle> m_ResourceLayouts;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spPipeline);

/// \brief A compute pipeline.
class SP_RHI_DLL spComputePipeline : public spPipeline
{
public:
  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsComputePipeline() const override { return true; }

protected:
  spComputePipeline(spComputePipelineDescription description)
    : spPipeline()
    , m_Description(std::move(description))
  {
  }

private:
  spComputePipelineDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spComputePipeline);
