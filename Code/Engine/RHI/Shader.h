#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Input.h>

namespace RHI
{
  /// \brief Describes a \a spShader, for creation using a \a spDeviceResourceFactory.
  struct spShaderDescription : ezHashableStruct<spShaderDescription>
  {
    /// \brief The compiled binary representation of the shader.
    ezConstByteArrayPtr m_Buffer;

    /// \brief The stage of the shader to create.
    ezEnum<spShaderStage> m_eShaderStage;

    /// \brief The name of the shader entry point.
    ezHashedString m_sEntryPoint;
  };

  struct spShaderPipeline : ezHashableStruct<spShaderPipeline>
  {
    ezDynamicArray<spResourceHandle> m_InputLayouts;
    spResourceHandle m_hShaderProgram{};

    spShaderPipeline() = default;

    spShaderPipeline(ezDynamicArray<spResourceHandle> inputLayouts, spResourceHandle hShaderProgram)
      : ezHashableStruct<spShaderPipeline>()
      , m_InputLayouts(std::move(inputLayouts))
      , m_hShaderProgram(hShaderProgram)
    {
    }
  };

  class SP_RHI_DLL spShaderProgram : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spShaderProgram, spDeviceResource);

  public:
    /// \brief Attaches the the given shader to the program.
    /// \param [in] pShader The handle to the shader to attach to the program.
    virtual void Attach(ezSharedPtr<spShader> pShader) = 0;

    /// \brief Detaches the given shader from the program.
    /// \param [in] pShader The handle to the shader to detach from the program.
    virtual void Detach(ezSharedPtr<spShader> pShader) = 0;

    /// \brief Detaches the shader attached at the given stage, if any.
    /// \param [in] eStage The stage to detach the shader from.
    virtual void Detach(const ezEnum<spShaderStage>& eStage) = 0;

    /// \brief Detaches all the attached shaders.
    virtual void DetachAll() = 0;

    /// \brief Sets the shader program as active in the current
    /// rendering or compute pipeline.
    virtual void Use() = 0;

    /// \brief Gets the handle to the shader resource associated with the program
    /// at the given stage.
    /// \param [in] eStage The shader stage.
    /// \return The shader resource handle. An invalid handle is returned if the given stage does not exist.
    EZ_NODISCARD virtual ezSharedPtr<spShader> Get(const ezEnum<spShaderStage>& eStage) const = 0;

    virtual void GetResourceLayoutDescriptions(ezDynamicArray<spResourceLayoutDescription>& out_resourceLayouts) const = 0;
  };

  class SP_RHI_DLL spShader : public spDeviceResource
  {
    friend class spDeviceResourceFactory;

    EZ_ADD_DYNAMIC_REFLECTION(spShader, spDeviceResource);

  public:
    /// \brief Gets the stage of this shader.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezEnum<spShaderStage> GetStage() const { return m_Description.m_eShaderStage; }

    /// \brief Get the name of the entry point function.
    EZ_NODISCARD EZ_ALWAYS_INLINE const ezHashedString& GetEntryPoint() const { return m_Description.m_sEntryPoint; }

  protected:
    explicit spShader(spShaderDescription description);

    spShaderDescription m_Description;
  };
} // namespace RHI
