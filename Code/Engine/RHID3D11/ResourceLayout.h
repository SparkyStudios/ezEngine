#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/ResourceLayout.h>

namespace RHI
{
  class spDeviceD3D11;

  class SP_RHID3D11_DLL spResourceLayoutD3D11 final : public spResourceLayout
  {
    friend class spDeviceResourceFactoryD3D11;

    EZ_ADD_DYNAMIC_REFLECTION(spResourceLayoutD3D11, spResourceLayout);

  public:
    // spDeviceResource

    void ReleaseResource() override;
    bool IsReleased() const override;

    // spResourceLayoutD3D11

    struct BindingInfo
    {
      ezUInt32 m_uiSlot{0};
      ezBitflags<spShaderStage> m_eShaderStage;
      ezEnum<spShaderResourceType> m_eResourceType;
      bool m_bDynamicBuffer{false};
    };

    spResourceLayoutD3D11(spDeviceD3D11* pDevice, const spResourceLayoutDescription& description);
    ~spResourceLayoutD3D11() override;

    EZ_NODISCARD BindingInfo GetBinding(ezUInt32 uiSlot) const;
    EZ_NODISCARD bool IsDynamicBuffer(ezUInt32 uiSlot) const;

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetConstantBufferCount() const { return m_uiConstantBufferCount; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetStorageBufferCount() const { return m_uiStorageBufferCount; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetTextureCount() const { return m_uiTextureCount; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetSamplerCount() const { return m_uiSamplerCount; }

  private:
    ezDynamicArray<BindingInfo> m_Bindings;

    ezUInt32 m_uiConstantBufferCount{0};
    ezUInt32 m_uiStorageBufferCount{0};
    ezUInt32 m_uiTextureCount{0};
    ezUInt32 m_uiSamplerCount{0};
  };
} // namespace RHI
