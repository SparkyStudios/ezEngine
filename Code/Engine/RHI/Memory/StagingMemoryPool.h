#pragma once

#include "Foundation/Containers/ArrayMap.h"
#include "Foundation/Containers/DynamicArray.h"
#include "Foundation/Memory/AllocatorBase.h"
#include <RHI/RHIDLL.h>

#include <Foundation/Containers/List.h>

/// \brief A single block in a staging memory pool.
struct SP_RHI_DLL spStagingMemoryBlock
{
  /// \brief The block ID.
  ezUInt32 m_uiId{0};

  /// \brief The pointer to the data stored in the block.
  void* m_pData{nullptr};

  /// \brief The requested size of the block in bytes.
  ezUInt32 m_uiSize{0};

  /// \brief The total block capacity in bytes.
  ezUInt32 m_uiCapacity{0};

  /// \brief Creates an empty staging memory block.
  spStagingMemoryBlock() = default;

  /// \brief Creates a new staging memory block.
  spStagingMemoryBlock(ezUInt32 uiId, void* pData, ezUInt32 uiSize, ezUInt32 uiCapacity)
    : m_uiId(uiId)
    , m_pData(pData)
    , m_uiSize(uiSize)
    , m_uiCapacity(uiCapacity)
  {
  }
};

class SP_RHI_DLL spStagingMemoryPool
{
  static constexpr const ezUInt32 kMinimumCapacity = 128;

public:
  spStagingMemoryPool(ezAllocatorBase* pAllocator);
  ~spStagingMemoryPool();

  spStagingMemoryBlock Stage(void* pData, ezUInt32 uiSize);

  spStagingMemoryBlock Stage(ezByteArrayPtr source);

  spStagingMemoryBlock GetBlock(ezUInt32 uiSize);

  spStagingMemoryBlock GetBlockWithId(ezUInt32 uiId);

  void Free(const spStagingMemoryBlock& block);

private:
  void Rent(ezUInt32 uiSize, spStagingMemoryBlock& block);

  void Allocate(ezUInt32 uiSize, spStagingMemoryBlock& block);

  typedef ezCompareHelper<ezUInt32> spStagingMemoryCapacityComparer;

  ezAllocatorBase* m_pAllocator;

  ezDynamicArray<spStagingMemoryBlock> m_Storage;
  ezArrayMap<ezUInt32, ezUInt32> m_AvailableMemoryBlocks;

  ezMutex m_Lock;
};
