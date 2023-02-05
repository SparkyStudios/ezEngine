#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Input.h>

/// \brief Describes a single shader specialization constant. Used to substitute new values into shaders when building
/// a \see spPipeline.
struct SP_RHI_DLL spShaderSpecializationConstant : ezHashableStruct<spShaderSpecializationConstant>
{
public:
  /// \brief The specialization constant ID, as defined in the shader.
  ezUInt32 m_uiId{0};

  /// \brief The type of data stored in the specialization constant. Must be a scalar type.
  ezEnum<spShaderSpecializationConstantType> m_eType{spShaderSpecializationConstantType::Default};

  /// \brief A 8-byte block containing the value of the specialization constant. This is threated as an
  /// untyped buffer and it's interpreted according to the type.
  ezUInt64 m_uiValue{0};

  /// \brief Constructs a new instance of spShaderSpecializationConstant.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] eType The type of data stored in the specialization constant.
  /// \param [in] uiData The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, ezEnum<spShaderSpecializationConstantType> eType, ezUInt64 uiData)
    : ezHashableStruct<spShaderSpecializationConstant>()
    , m_uiId(uiId)
    , m_eType(eType)
    , m_uiValue(uiData)
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for a boolean value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] bValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, bool bValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::Bool, Store(bValue ? 1u : 0u))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for an unsigned short integer value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] uiValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, ezUInt16 uiValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::UInt16, Store(uiValue))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for a short integer value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] iValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, ezInt16 iValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::Int16, Store(iValue))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for an unsigned integer value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] uiValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, ezUInt32 uiValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::UInt32, Store(uiValue))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for an integer value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] iValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, ezInt32 iValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::Int32, Store(iValue))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for an unsigned long integer value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] uiValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, ezUInt64 uiValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::UInt64, Store(uiValue))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for a long integer value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] iValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, ezInt64 iValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::Int64, Store(iValue))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for a float value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] fValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, float fValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::Float, Store(fValue))
  {
  }

  /// \brief Constructs a new instance of spShaderSpecializationConstant for a double value.
  /// \param [in] uiId The ID of the specialization constant.
  /// \param [in] dValue The value of the specialization constant.
  spShaderSpecializationConstant(ezUInt32 uiId, double dValue)
    : spShaderSpecializationConstant(uiId, spShaderSpecializationConstantType::Double, Store(dValue))
  {
  }

private:
  template <typename T>
  static ezUInt64 Store(T value)
  {
    ezUInt64 uiValue = 0;
    memcpy(&uiValue, &value, sizeof(T));
    return uiValue;
  }
};

/// \brief Describes a \see spShader, for creation using a \see spDeviceResourceFactory.
struct spShaderDescription : public ezHashableStruct<spShaderDescription>
{
  /// \brief The compiled binary representation of the shader.
  ezByteArrayPtr m_Buffer;

  /// \brief The stage of the shader to create.
  ezEnum<spShaderStage> m_eShaderStage;

  /// \brief The name of the shader entry point.
  ezHashedString m_sEntryPoint;
};

struct SP_RHI_DLL spShaderPipeline : public ezHashableStruct<spShaderPipeline>
{
  ezDynamicArray<spResourceHandle> m_InputLayouts;
  spResourceHandle m_hVertexShader{};
  spResourceHandle m_hPixelShader{};
  spResourceHandle m_hGeometryShader{};
  spResourceHandle m_hHullShader{};
  spResourceHandle m_hDomainShader{};
  spResourceHandle m_hComputeShader{};

  spShaderPipeline() = default;

  spShaderPipeline(
    ezDynamicArray<spResourceHandle> inputLayouts,
    spResourceHandle hVertexShader,
    spResourceHandle hPixelShader,
    spResourceHandle hGeometryShader = spResourceHandle(),
    spResourceHandle hDomainShader = spResourceHandle(),
    spResourceHandle hHullShader = spResourceHandle());

  /// \brief Constructs a \see spShaderPipeline for a \see spComputePipeline.
  spShaderPipeline(
    ezDynamicArray<spResourceHandle> inputLayouts,
    spResourceHandle hComputeShader);

  ~spShaderPipeline();
};

class SP_RHI_DLL spShaderProgram : public spDeviceResource
{
public:
  /// \brief Attaches the the given shader to the program.
  /// \param [in] hShader The handle to the shader to attach to the program.
  virtual void Attach(const spResourceHandle& hShader) = 0;

  /// \brief Detaches the given shader from the program.
  /// \param [in] hShader The handle to the shader to detach from the program.
  virtual void Detach(const spResourceHandle& hShader) = 0;

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
  EZ_NODISCARD virtual spResourceHandle Get(const ezEnum<spShaderStage>& eStage) const = 0;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spShaderProgram);

class SP_RHI_DLL spShader : public spDeviceResource
{
  friend class spDeviceResourceFactory;

public:
  /// \brief Gets the stage of this shader.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezEnum<spShaderStage> GetStage() const { return m_Description.m_eShaderStage; }

  /// \brief Get the name of the entry point function.
  EZ_NODISCARD EZ_ALWAYS_INLINE const ezHashedString& GetEntryPoint() const { return m_Description.m_sEntryPoint; }

protected:
  spShader(spShaderDescription description)
    : m_Description(std::move(description))
  {
  }

  spShaderDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spShader);
