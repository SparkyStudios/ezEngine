// Copyright (c) 2024-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/Buffer.h>

namespace RPI
{
  template <typename T, RHI::spBufferUsage::Enum Usage>
  class spConstantBufferView;

  /// \brief Base class for constant buffer storages.
  class SP_RPI_DLL spConstantBufferBase
  {
  public:
    /// \brief Gets the RHI buffer resource.
    EZ_ALWAYS_INLINE ezSharedPtr<RHI::spBuffer> GetBuffer() const { return m_pBuffer; }

    /// \brief Maps the GPU buffer to CPU memory.
    /// \param[in] bReadOnly Specifies whether the buffer should be mapped as read-only.
    /// \return The mapped resource.
    const RHI::spMappedResource& Map(bool bReadOnly = false) const;

    /// \brief Unmaps the GPU buffer from CPU memory.
    void Unmap() const;

    /// \brief Swaps the inner GPU buffer ranges and activates the next one.
    /// This method works only if the buffer is multi-buffered.
    void Swap() const;

    /// \brief Updates the GPU buffer with the given data.
    /// \param[in] uiOffsetInBytes The offset in the buffer to write to.
    /// \param[in] uiSizeInBytes The number of bytes to write.
    /// \param[in] pData The data to write.
    void UpdateBuffer(ezUInt32 uiOffsetInBytes, ezUInt32 uiSizeInBytes, const void* pData) const;

    /// \brief Gets the handle to the buffer resource.
    EZ_NODISCARD RHI::spResourceHandle GetHandle() const;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    /// \brief Sets the debug name of the buffer.
    /// \param[in] sName The name to set.
    /// \note This method is only available in debug builds.
    void SetDebugName(ezStringView sName) const;
#endif

  protected:
    /// \brief Creates a new constant buffer wrapper with the given size.
    /// \param[in] uiSizeInBytes The size of the buffer.
    /// \param[in] eUsage The usage of the buffer.
    spConstantBufferBase(ezUInt32 uiSizeInBytes, ezBitflags<RHI::spBufferUsage> eUsage);

    /// \brief Destroys the constant buffer wrapper and frees all allocated resources.
    virtual ~spConstantBufferBase();

    ezSharedPtr<RHI::spBuffer> m_pBuffer{nullptr};
  };

  /// \brief Constant buffer storage that is mapped to CPU memory.
  ///
  /// This class is intended to be used in places where data are needed to be read from the GPU and the CPU at the same
  /// time. It uses native CPU memory mapping from the RHI device. Memory can be mapped as read-only or write-only, but
  /// never as read-write. You cannot map a buffer as read-only and write-only at the same time.
  ///
  /// \tparam T The data type of the buffer. It's recommended to be a struct with 16-bit alignment.
  template <typename T, RHI::spBufferUsage::Enum Usage = RHI::spBufferUsage::Dynamic>
  class spConstantBuffer final : public spConstantBufferBase, public ezRefCounted
  {
    friend class spConstantBufferView<T, Usage>;
    friend class spConstantBufferView<const T, Usage>;

  public:
    static constexpr ezUInt64 DataSize = sizeof(T);

    /// \brief Create a new constant buffer storage.
    spConstantBuffer()
      : spConstantBufferBase(DataSize, Usage)
    {
      m_RawData = EZ_DEFAULT_NEW_ARRAY(ezUInt8, DataSize);
      ezMemoryUtils::ZeroFill(m_RawData.GetPtr(), m_RawData.GetCount());
    }

    /// \brief Create a new constant buffer storage with the given value.
    /// \param[in] value The value to initialize the buffer with.
    explicit spConstantBuffer(const T& value)
      : spConstantBuffer()
    {
      Set(value);
    }

    ~spConstantBuffer() override
    {
      EZ_DEFAULT_DELETE_ARRAY(m_RawData);
    }

    /// \brief Get a view to the constant buffer for writing.
    /// \note The view will automatically unmap the buffer when it goes out of scope.
    /// \return A view to the constant buffer for writing.
    EZ_FORCE_INLINE spConstantBufferView<T, Usage> Write() const
    {
      return spConstantBufferView<T, Usage>(this);
    }

    /// \brief Get a view to the constant buffer for reading.
    /// \note The view will automatically unmap the buffer when it goes out of scope.
    /// \return A view to the constant buffer for reading.
    EZ_FORCE_INLINE spConstantBufferView<const T, Usage> Read() const
    {
      return spConstantBufferView<const T, Usage>(this);
    }

    /// \brief Sets the value of the constant buffer.
    /// \param[in] value The value to set in the buffer.
    EZ_FORCE_INLINE void Set(const T& value) const
    {
      ezMemoryUtils::RawByteCopy(m_RawData.GetPtr(), &value, DataSize);
    }

    /// \brief Gets the hash of the constant buffer's value.
    EZ_NODISCARD EZ_FORCE_INLINE ezUInt32 GetHash() const
    {
      return ezHashingUtils::xxHash32(m_RawData.GetPtr(), m_RawData.GetCount());
    }

  private:
    ezByteArrayPtr m_RawData;
  };

  /// \brief A view to a constant buffer storage.
  ///
  /// This class allow to access the data of a constant buffer storage in a read-only or write-only manner.
  /// The buffer is guaranteed to be unmapped when the view goes out of scope.
  ///
  /// \tparam T The data type of the buffer. It's recommended to be a struct with 16-bit alignment.
  template <typename T, RHI::spBufferUsage::Enum Usage = RHI::spBufferUsage::Dynamic>
  class spConstantBufferView
  {
  public:
    explicit spConstantBufferView(const spConstantBuffer<std::remove_const_t<T>, Usage>* pBuffer)
    {
      m_pBuffer = pBuffer;
      m_uiDataHash = pBuffer->GetHash();
    }

    ~spConstantBufferView()
    {
      if (std::is_const_v<T>)
        return;

      if (const ezUInt32 uiDataHash = m_pBuffer->GetHash(); uiDataHash == m_uiDataHash)
        return;

      {
        const RHI::spMappedResource resource = m_pBuffer->Map();

        ezMemoryUtils::RawByteCopy(
          static_cast<ezUInt8*>(resource.GetData()) + m_pBuffer->m_pBuffer->GetCurrentRange()->GetOffset(),
          m_pBuffer->m_RawData.GetPtr(),
          m_pBuffer->m_RawData.GetCount());

        m_pBuffer->Unmap();
      }
    }

    EZ_FORCE_INLINE T* operator->() const
    {
      return reinterpret_cast<T*>(m_pBuffer->m_RawData.GetPtr());
    }

    EZ_FORCE_INLINE T& operator*() const
    {
      return *reinterpret_cast<T*>(m_pBuffer->m_RawData.GetPtr());
    }

    EZ_FORCE_INLINE T& operator=(const T& value) const
    {
      m_pBuffer->Set(value);
      return *reinterpret_cast<T*>(m_pBuffer->m_RawData.GetPtr());
    }

  private:
    ezUInt32 m_uiDataHash{0};
    const spConstantBuffer<std::remove_const_t<T>, Usage>* m_pBuffer{nullptr};
  };
} // namespace RPI
