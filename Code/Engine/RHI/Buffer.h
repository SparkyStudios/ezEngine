#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Fence.h>
#include <RHI/Resource.h>

namespace RHI
{
  /// \brief Describes a Buffer, for creation of buffer objects with a \a spDeviceResourceFactory.
  struct spBufferDescription : ezHashableStruct<spBufferDescription>
  {
    spBufferDescription() = default;

    /// \brief Constructs a \a spBufferDescription for a non-dynamic buffer.
    /// \param uiSize The size of the buffer in bytes.
    /// \param eUsage The desired usage of the buffer.
    spBufferDescription(ezUInt32 uiSize, const ezBitflags<spBufferUsage>& eUsage)
      : ezHashableStruct()
    {
      m_uiSize = uiSize;
      m_eUsage = eUsage;
      m_uiStructureStride = 0;
      m_bRawBuffer = false;
    }

    /// \brief Constructs a \a spBufferDescription.
    /// \param uiSize The size in bytes of the buffer.
    /// \param eUsage The desired usage of the buffer.
    /// \param uiStructureStride The size in bytes of a single element in a structured buffer. 0 for non-structured buffers.
    /// \param bRawBuffer Whether the buffer is a raw buffer.
    spBufferDescription(ezUInt32 uiSize, const ezBitflags<spBufferUsage>& eUsage, ezUInt32 uiStructureStride, bool bRawBuffer = false)
      : ezHashableStruct()
    {
      m_uiSize = uiSize;
      m_eUsage = eUsage;
      m_uiStructureStride = uiStructureStride;
      m_bRawBuffer = bRawBuffer;
    }

    /// \brief Compares this instance with the given \a other instance for equality.
    EZ_ALWAYS_INLINE bool operator==(const spBufferDescription& other) const
    {
      return m_uiSize == other.m_uiSize && m_eUsage == other.m_eUsage && m_uiStructureStride == other.m_uiStructureStride && m_bRawBuffer == other.m_bRawBuffer;
    }

    /// \brief Compares this instance with the given \a other instance for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spBufferDescription& other) const
    {
      return !(*this == other);
    }

    /// \brief The desired capacity, in bytes, of the buffer.
    ezUInt32 m_uiSize{0};

    /// \brief Indicates how the buffer will be allocated and used.
    ezBitflags<spBufferUsage> m_eUsage;

    /// \brief For structured buffers, indicates the size in bytes of a single element,
    /// and must be non-zero. For all other buffers types, the size must be zero.
    ezUInt32 m_uiStructureStride{0};

    /// \brief Whether this buffer is a raw buffer. This should be combined with
    /// \a spBufferUsage::StructuredBufferReadWrite. This affects how the buffer is bound
    /// in the D3D11 backend.
    bool m_bRawBuffer{false};
  };

  /// \brief Describes a section of a buffer. This can be used in place of a buffer to make only a subset of it
  /// available to the GPU.
  struct spBufferRangeDescription : ezHashableStruct<spBufferRangeDescription>
  {
    /// \brief Constructs a new buffer range description.
    /// \param hBuffer The parent buffer.
    /// \param uiOffset The range offset in bytes.
    /// \param uiSize The range size in bytes.
    /// \param fence The synchronization fence of this buffer range.
    spBufferRangeDescription(spResourceHandle hBuffer, ezUInt32 uiOffset, ezUInt32 uiSize, spFenceDescription fence)
      : ezHashableStruct()
    {
      m_hBuffer = hBuffer;
      m_uiOffset = uiOffset;
      m_uiSize = uiSize;
      m_Fence = std::move(fence);
    }

    /// \brief Constructs a new buffer range description.
    /// \param hBuffer The parent buffer.
    /// \param uiOffset The range offset in bytes.
    /// \param uiSize The range size in bytes.
    spBufferRangeDescription(spResourceHandle hBuffer, ezUInt32 uiOffset, ezUInt32 uiSize)
      : ezHashableStruct()
    {
      m_hBuffer = hBuffer;
      m_uiOffset = uiOffset;
      m_uiSize = uiSize;
    }

    /// \brief Compares this buffer range description with an \a other buffer range description for equality.
    EZ_ALWAYS_INLINE bool operator==(const spBufferRangeDescription& other) const
    {
      return m_hBuffer == other.m_hBuffer && m_uiOffset == other.m_uiOffset && m_uiSize == other.m_uiSize && m_Fence == other.m_Fence;
    }

    /// \brief Compares this buffer range description with an \a other buffer range description for inequality
    EZ_ALWAYS_INLINE bool operator!=(const spBufferRangeDescription& other) const
    {
      return !(*this == other);
    }

    /// \brief The parent buffer in which this range is created.
    spResourceHandle m_hBuffer;

    /// \brief The offset in bytes, from the beginning of the buffer that this range belongs to.
    ezUInt32 m_uiOffset;

    /// \brief The size in bytes of the range.
    ezUInt32 m_uiSize;

    /// \brief The descriptor of the fence which will control synchronization on this buffer range.
    spFenceDescription m_Fence;
  };

  /// \brief A range in a spBuffer.
  class SP_RHI_DLL spBufferRange : public spShaderResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spBufferRange, spShaderResource);

  public:
    /// \brief Gets the handle to the parent buffer of this range.
    [[nodiscard]] EZ_ALWAYS_INLINE virtual ezSharedPtr<spBuffer> GetBuffer() const = 0;

    /// \brief Gets the offset in bytes from which stats this range in the parent buffer.
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetOffset() const { return m_Description.m_uiOffset; }

    /// \brief Gets the size of this range.
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetSize() const { return m_Description.m_uiSize; }

    /// \brief Gets the handle of the \see spFence which will control synchronization on this buffer range.
    [[nodiscard]] virtual ezSharedPtr<spFence> GetFence() const = 0;

  protected:
    explicit spBufferRange(spBufferRangeDescription description);

    spBufferRangeDescription m_Description;
  };

  /// \brief A set of data in the memory, readable and writable from both CPU and GPU.
  class SP_RHI_DLL spBuffer : public spMappableResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spBuffer, spMappableResource);

    friend class spDeviceResourceFactory;

  public:
    /// \brief Gets the size in bytes of the buffer. In case of multi-buffered buffers, this
    /// is the size of a single buffer.
    [[nodiscard]] EZ_ALWAYS_INLINE virtual ezUInt32 GetSize() const { return m_Description.m_uiSize; }

    /// \brief Gets the stride of the structure in bytes.
    [[nodiscard]] EZ_ALWAYS_INLINE virtual ezUInt32 GetStructureStride() const { return m_Description.m_uiStructureStride; }

    /// \brief Gets the usage of the buffer.
    [[nodiscard]] EZ_ALWAYS_INLINE virtual ezBitflags<spBufferUsage> GetUsage() const { return m_Description.m_eUsage; }

    /// \brief Checks whether the buffer is dynamic.
    [[nodiscard]] EZ_ALWAYS_INLINE bool IsDynamic() const { return GetUsage().IsAnySet(spBufferUsage::Dynamic | spBufferUsage::PersistentMapping); }

    /// \brief Checks whether the buffer is a raw buffer.
    [[nodiscard]] EZ_ALWAYS_INLINE bool IsRawBuffer() const { return m_Description.m_bRawBuffer; }

    /// \brief Gets the index of the currently bound buffer range in case of multi-buffered buffers.
    /// \note This always returns 0 in case of single-buffered buffers.
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetCurrentRangeIndex() const { return m_uiBufferIndex; }

    /// \brief Gets the currently used buffer range.
    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<spBufferRange> GetCurrentRange() const { return m_BufferRanges[m_uiBufferIndex]; }

    /// \brief Gets the buffer ranges stored by this buffer.
    ///
    /// The number of elements in the returned array depends on the buffering level of the buffer.
    /// - For a single-buffered buffer, there is only one range covering all the buffer.
    /// - For a double-buffered buffer, there are two ranges.
    /// - For a triple-buffered buffer, there are three ranges.
    ///
    /// You can use the result of GetBufferIndex() to determine the range of the buffer currently being used.
    [[nodiscard]] EZ_ALWAYS_INLINE const ezStaticArray<ezSharedPtr<spBufferRange>, SP_RHI_MAX_BUFFERING_LEVEL>& GetBufferRanges() const { return m_BufferRanges; }

    /// \brief Swap buffers in case of multi-buffering. This method is a no-op if the buffer is single-buffered.
    EZ_ALWAYS_INLINE void SwapBuffers() { m_uiBufferIndex = (m_uiBufferIndex + 1) % m_uiBufferCount; }

  protected:
    explicit spBuffer(spBufferDescription description);

    void PreCreateResource();
    void PostCreateResource();

    spBufferDescription m_Description;

    ezUInt32 m_uiBufferIndex{0};
    ezUInt32 m_uiBufferCount{1};
    ezUInt32 m_uiBufferAlignedSize{0};

    ezStaticArray<ezSharedPtr<spBufferRange>, SP_RHI_MAX_BUFFERING_LEVEL> m_BufferRanges;
  };
} // namespace RHI
