#include <RHI/RHIPCH.h>

#include <RHI/Memory/StagingMemoryPool.h>

spStagingMemoryPool::spStagingMemoryPool(ezAllocatorBase* pAllocator)
  : m_pAllocator(pAllocator)
  , m_Storage()
  , m_AvailableMemoryBlocks()
  , m_Lock()
{
}

spStagingMemoryPool::~spStagingMemoryPool()
{
  EZ_LOCK(m_Lock);

  m_AvailableMemoryBlocks.Clear();

  for (const auto& it : m_Storage)
    m_pAllocator->Deallocate(it.m_pData);

  m_Storage.Clear();
}

spStagingMemoryBlock spStagingMemoryPool::Stage(void* pData, ezUInt32 uiSize)
{
  spStagingMemoryBlock block;
  Rent(uiSize, block);
  memcpy(block.m_pData, pData, uiSize);
  return block;
}

spStagingMemoryBlock spStagingMemoryPool::Stage(ezByteArrayPtr source)
{
  spStagingMemoryBlock block;
  Rent(source.GetCount(), block);
  memcpy(block.m_pData, source.GetPtr(), source.GetCount());
  return block;
}

spStagingMemoryBlock spStagingMemoryPool::GetBlock(ezUInt32 uiSize)
{
  spStagingMemoryBlock block;
  Rent(uiSize, block);
  return block;
}

spStagingMemoryBlock spStagingMemoryPool::GetBlockWithId(ezUInt32 uiId)
{
  EZ_LOCK(m_Lock);

  for (const auto& it : m_Storage)
    if (it.m_uiId == uiId)
      return it;

  return {};
}

void spStagingMemoryPool::Rent(ezUInt32 uiSize, spStagingMemoryBlock& block)
{
  EZ_LOCK(m_Lock);

  for (ezUInt32 i = 0, l = m_AvailableMemoryBlocks.GetCount(); i < l; ++i)
  {
    ezUInt32 index = m_AvailableMemoryBlocks.GetValue(i);
    spStagingMemoryBlock current = m_Storage[index];

    if (current.m_uiCapacity < uiSize)
      continue;

    m_AvailableMemoryBlocks.RemoveAtAndCopy(i, true);
    current.m_uiSize = uiSize;
    block = current;

    m_Storage[index] = current;

    return;
  }

  Allocate(uiSize, block);
}

void spStagingMemoryPool::Allocate(ezUInt32 uiSize, spStagingMemoryBlock& block)
{
  ezUInt32 uiCapacity = ezMath::Max(kMinimumCapacity, uiSize);
  void* pData = m_pAllocator->Allocate(uiCapacity, 16);
  ezUInt32 uiId = m_Storage.GetCount();
  block = spStagingMemoryBlock(uiId, pData, uiSize, uiCapacity);
  m_Storage.PushBack(block);
}

void spStagingMemoryPool::Free(const spStagingMemoryBlock& block)
{
  EZ_LOCK(m_Lock);

  EZ_ASSERT_DEV(block.m_uiId < m_Storage.GetCount(), "The memory block is out of range.");
  m_AvailableMemoryBlocks.Insert(block.m_uiCapacity, block.m_uiId);
}

EZ_STATICLINK_FILE(RHI, RHI_Memory_Implementation_StagingMemoryPool);
