#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/ResourceLayout.h>

namespace RHI
{
  class spDeviceMTL;

  class SP_RHIMTL_DLL spResourceLayoutMTL final : public spResourceLayout
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spResourceLayoutMTL, spResourceLayout);

  public:
    // spDeviceResource

    void ReleaseResource() override;
    bool IsReleased() const override;

    // spResourceLayoutMTL

    struct BindingInfo
    {
      ezUInt32 m_uiSlot{0};
      ezUInt32 m_uiResourceSlot{0};
      ezBitflags<spShaderStage> m_eShaderStage;
      ezEnum<spShaderResourceType> m_eResourceType;
      bool m_bDynamicBuffer{false};
    };

    spResourceLayoutMTL(spDeviceMTL* pDevice, const spResourceLayoutDescription& description);
    ~spResourceLayoutMTL() override;

    EZ_NODISCARD const BindingInfo& GetBinding(ezUInt32 uiSlot) const;
    EZ_NODISCARD bool IsDynamicBuffer(ezUInt32 uiSlot) const;

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetBufferCount() const { return m_uiBufferCount; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetTextureCount() const { return m_uiTextureCount; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetSamplerCount() const { return m_uiSamplerCount; }

  private:
    ezDynamicArray<BindingInfo> m_Bindings;

    ezUInt32 m_uiBufferCount{0};
    ezUInt32 m_uiTextureCount{0};
    ezUInt32 m_uiSamplerCount{0};
  };
} // namespace RHI
