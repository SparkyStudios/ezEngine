#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Buffer.h>

#include <RHID3D11/Fence.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spBufferD3D11 final : public spBuffer, public spDeferredDeviceResource
{
  friend class spDeviceResourceFactoryD3D11;

public:
  ~spBufferD3D11() override;

  void SetDebugName(const ezString& name) override;

  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsReleased() const override { return m_pBuffer == nullptr; }

  void ReleaseResource() override;

  void CreateResource() override;

  /// \brief Gets the wrapped native buffer.
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11Buffer* GetD3D11Buffer() const { return m_pBuffer; }

  /// \brief Gets a shader resource view of the buffer for the given offset and size.
  /// \param uiOffset The offset of the resource to get.
  /// \param uiSize The size of the resource to get.
  ID3D11ShaderResourceView* GetShaderResourceView(ezUInt32 uiOffset, ezUInt32 uiSize);

  /// \brief Gets an unordered access view of the buffer for the given offset and size.
  /// \param uiOffset The offset of the resource to get.
  /// \param uiSize The size of the resource to get.
  ID3D11UnorderedAccessView* GetUnorderedAccessView(ezUInt32 uiOffset, ezUInt32 uiSize);

private:
  spBufferD3D11(spDeviceD3D11* pDevice, const spBufferDescription& description);

  struct OffsetSizePair
  {
    OffsetSizePair() = default;

    OffsetSizePair(ezUInt32 uiOffset, ezUInt32 uiSize)
      : m_uiOffset(uiOffset)
      , m_uiSize(uiSize)
    {
    }

    EZ_ALWAYS_INLINE bool operator<(const OffsetSizePair& other) const
    {
      return m_uiOffset + m_uiSize < other.m_uiOffset + other.m_uiSize;
    }

    EZ_ALWAYS_INLINE bool operator==(const OffsetSizePair& other) const
    {
      return m_uiOffset == other.m_uiOffset && m_uiSize == other.m_uiSize;
    }

    EZ_ALWAYS_INLINE bool operator!=(const OffsetSizePair& other) const
    {
      return !(*this == other);
    }

    ezUInt32 m_uiOffset{0};
    ezUInt32 m_uiSize{0};
  };

  ID3D11ShaderResourceView* CreateShaderResourceView(ezUInt32 uiOffset, ezUInt32 uiSize) const;
  ID3D11UnorderedAccessView* CreateUnorderedAccessView(ezUInt32 uiOffset, ezUInt32 uiSize) const;

  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11Buffer* m_pBuffer{nullptr};

  ezMap<OffsetSizePair, ID3D11ShaderResourceView*> m_pShaderResourceViews;
  ezMap<OffsetSizePair, ID3D11UnorderedAccessView*> m_pUnorderedAccessViews;

  ezMutex m_AccessViewLock;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spBufferD3D11);

class SP_RHID3D11_DLL spBufferRangeD3D11 final : public spBufferRange
{
  friend class spDeviceResourceFactoryD3D11;

  // spDeviceResource

public:
  void ReleaseResource() override;
  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsReleased() const override { return m_bReleased; }

  // spBufferRange

public:
  EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spFence> GetFence() const override { return m_pFence; }

  // spBufferRangeD3D11

public:
  spBufferRangeD3D11(spDeviceD3D11* pDevice, const spBufferRangeDescription& description);
  ~spBufferRangeD3D11() override;

  /// \brief Checks if the buffer range is valid.
  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsValid() const { return m_pBuffer != nullptr; }

  /// \brief Checks if the buffer range is covering the entire parent buffer.
  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsFullRange() const { return IsValid() && GetOffset() == 0 && GetSize() == m_pBuffer->GetSize(); }

  /// \brief Gets the parent buffer.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spBuffer> GetBuffer() const override { return m_pBuffer; }

  EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const spBufferRangeD3D11& rhs) const
  {
    return m_pBuffer == rhs.m_pBuffer && GetOffset() == rhs.GetOffset() && GetSize() == rhs.GetSize();
  }

  EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const spBufferRangeD3D11& rhs) const { return !(*this == rhs); }

private:
  ezSharedPtr<spBufferD3D11> m_pBuffer{nullptr};
  ezSharedPtr<spFenceD3D11> m_pFence{nullptr};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spBufferRangeD3D11);
