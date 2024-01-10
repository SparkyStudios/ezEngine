#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Buffer.h>
#include <RHI/Device.h>

#include <RHIMTL/Fence.h>

namespace RHI
{
  class spDeviceMTL;

  class SP_RHIMTL_DLL spBufferMTL final : public spBuffer, public spDeferredDeviceResource
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spBufferMTL, spBuffer);

  public:
    ~spBufferMTL() override;

    void SetDebugName(ezStringView sDebugName) override;

    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsReleased() const override { return m_pBuffer == nullptr; }

    void ReleaseResource() override;

    void CreateResource() override;

    /// \brief Gets the native wrapped buffer.
    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::Buffer* GetMTLBuffer() const { return m_pBuffer; }

  private:
    spBufferMTL(spDeviceMTL* pDevice, const spBufferDescription& description);

    ezUInt32 m_uiActualCapacity{0};

    MTL::Device* m_pMTLDevice{nullptr};
    MTL::Buffer* m_pBuffer{nullptr};
  };

  class SP_RHIMTL_DLL spBufferRangeMTL final : public spBufferRange, public spDeferredDeviceResource
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spBufferRangeMTL, spBufferRange);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsReleased() const override { return m_bReleased; }

    // spBufferRange

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spFence> GetFence() const override { return m_pFence; }

    // spDeferredDeviceResource

  public:
    void CreateResource() override;

    // spBufferRangeMTL

  public:
    spBufferRangeMTL(spDeviceMTL* pDevice, const spBufferRangeDescription& description);
    ~spBufferRangeMTL() override;

    /// \brief Checks if the buffer range is valid.
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsValid() const { return m_pBuffer != nullptr; }

    /// \brief Checks if the buffer range is covering the entire parent buffer.
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsFullRange() const { return IsValid() && GetOffset() == 0 && GetSize() == m_pBuffer->GetSize(); }

    /// \brief Gets the parent buffer.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spBuffer> GetBuffer() const override { return {m_pBuffer, m_pDevice->GetAllocator()}; }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const spBufferRangeMTL& rhs) const
    {
      return m_pBuffer == rhs.m_pBuffer && GetOffset() == rhs.GetOffset() && GetSize() == rhs.GetSize();
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const spBufferRangeMTL& rhs) const { return !(*this == rhs); }

  private:
    spBufferMTL* m_pBuffer{nullptr};
    ezSharedPtr<spFenceMTL> m_pFence{nullptr};
  };
} // namespace RHI
