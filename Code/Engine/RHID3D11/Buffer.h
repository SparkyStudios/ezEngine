#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <d3d11.h>

#include <Foundation/Threading/Mutex.h>

#include <RHI/Buffer.h>

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
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

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

  ID3D11ShaderResourceView* GetShaderResourceView(ezUInt32 uiOffset, ezUInt32 uiSize);
  ID3D11UnorderedAccessView* GetUnorderedAccessView(ezUInt32 uiOffset, ezUInt32 uiSize);

  ID3D11ShaderResourceView* CreateShaderResourceView(ezUInt32 uiOffset, ezUInt32 uiSize) const;
  ID3D11UnorderedAccessView* CreateUnorderedAccessView(ezUInt32 uiOffset, ezUInt32 uiSize) const;

private:
  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11Buffer* m_pBuffer{nullptr};

  ezMap<OffsetSizePair, ID3D11ShaderResourceView*> m_pShaderResourceViews;
  ezMap<OffsetSizePair, ID3D11UnorderedAccessView*> m_pUnorderedAccessViews;

  ezMutex m_AccessViewLock;

  ezString m_sName;
};
