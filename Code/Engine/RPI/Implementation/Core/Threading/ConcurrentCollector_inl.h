// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

namespace RPI
{
  template <typename T, typename TAllocatorWrapper>
  spConcurrentCollector<T, TAllocatorWrapper>::spConcurrentCollector(ezUInt32 uiInitialCapacity)
  {
    m_pTail = m_pHead = EZ_NEW(TAllocatorWrapper::GetAllocator(), Segment, uiInitialCapacity);
  }

  template <typename T, typename TAllocatorWrapper>
  spConcurrentCollector<T, TAllocatorWrapper>::~spConcurrentCollector()
  {
    Segment* pSegment = m_pHead;
    while (pSegment != nullptr)
    {
      pSegment->m_Items.Clear();
      Segment* pNext = pSegment->m_pNext;
      EZ_DELETE(TAllocatorWrapper::GetAllocator(), pSegment);
      pSegment = pNext;
    }
  }

  template <typename T, typename TAllocatorWrapper>
  void spConcurrentCollector<T, TAllocatorWrapper>::Close()
  {
    if (m_pHead->m_pNext == nullptr)
      return;

    ezStaticArray<T, 16> newItems;
    newItems.SetCountUninitialized(m_pTail->m_uiOffset + m_pTail->m_Items.GetCount());

    Segment* pSegment = m_pHead;
    while (pSegment != nullptr)
    {
      ezMemoryUtils::Copy(newItems.GetData() + pSegment->m_uiOffset, pSegment->m_Items.GetData(), pSegment->m_Items.GetCount());

      pSegment->m_Items.Clear();
      Segment* pNext = pSegment->m_pNext;

      if (pSegment != m_pHead)
        EZ_DELETE(TAllocatorWrapper::GetAllocator(), pSegment);

      pSegment = pNext;
    }

    m_pHead->m_Items = newItems;
    m_pHead->m_pNext = nullptr;

    m_pTail = m_pHead;
  }

  template <typename T, typename TAllocatorWrapper>
  ezInt32 spConcurrentCollector<T, TAllocatorWrapper>::Add(const T& item)
  {
    const ezInt32 uiIndex = m_uiCount.Increment();

    Segment* pSegment = m_pTail;
    if (uiIndex >= pSegment->m_uiOffset + pSegment->m_Items.GetCount())
    {
      EZ_LOCK(m_ResizeMutex);

      if (uiIndex >= m_pTail->m_uiOffset + m_pTail->m_Items.GetCount())
      {
        m_pTail->m_pNext = EZ_NEW(TAllocatorWrapper::GetAllocator(), Segment, pSegment->m_Items.GetCount() * 2);
        m_pTail->m_pNext->m_uiOffset = pSegment->m_uiOffset + pSegment->m_Items.GetCount();
        m_pTail->m_pPrev = m_pTail;

        m_pTail = m_pTail->m_pNext;
      }

      pSegment = m_pTail;
    }

    while (uiIndex < pSegment->m_uiOffset)
      pSegment = pSegment->m_pPrev;

    pSegment->m_Items[uiIndex - pSegment->m_uiOffset] = item;

    return uiIndex;
  }

  template <typename T, typename TAllocatorWrapper>
  void spConcurrentCollector<T, TAllocatorWrapper>::AddRange(const ezArrayPtr<T>& items)
  {
    const ezInt32 uiNewCount = m_uiCount.Add(items.GetCount());

    Segment* pSegment = m_pTail;
    if (uiNewCount >= pSegment->m_uiOffset + pSegment->m_Items.GetCount())
    {
      EZ_LOCK(m_ResizeMutex);

      if (uiNewCount >= m_pTail->m_uiOffset + m_pTail->m_Items.GetCount())
      {
        const ezUInt32 uiNewCapacity = m_pTail->m_uiOffset + m_pTail->m_Items.GetCount();
        const ezUInt32 uiSize = ezMath::Max(uiNewCapacity, uiNewCount - uiNewCapacity);

        m_pTail->m_pNext = EZ_NEW(TAllocatorWrapper::GetAllocator(), Segment, uiSize);
        m_pTail->m_pNext->m_uiOffset = uiNewCapacity;
        m_pTail->m_pPrev = m_pTail;

        m_pTail = m_pTail->m_pNext;
      }
    }

    while (uiNewCount <= pSegment->m_uiOffset)
      pSegment = pSegment->m_pPrev;

    ezInt32 uiDestinationIndex = uiNewCount - pSegment->m_uiOffset - 1;
    for (ezInt32 sourceIndex = items.GetCount() - 1; sourceIndex >= 0; --sourceIndex)
    {
      if (uiDestinationIndex < 0)
      {
        pSegment = pSegment->m_pPrev;
        uiDestinationIndex = pSegment->m_Items.GetCount() - 1;
      }

      pSegment->m_Items[uiDestinationIndex--] = items[sourceIndex];
    }
  }

  template <typename T, typename TAllocatorWrapper>
  void spConcurrentCollector<T, TAllocatorWrapper>::Clear()
  {
    Close();
    GetItems().Clear();
    m_uiCount.Set(0);
  }

  template <typename T, typename TAllocatorWrapper>
  EZ_ALWAYS_INLINE const T& spConcurrentCollector<T, TAllocatorWrapper>::operator[](ezUInt32 uiIndex) const
  {
    EZ_ASSERT_DEBUG(uiIndex < m_uiCount, "Index out of bounds");
    return GetItems()[uiIndex];
  }

  template <typename T, typename TAllocatorWrapper>
  EZ_ALWAYS_INLINE T& spConcurrentCollector<T, TAllocatorWrapper>::operator[](ezUInt32 uiIndex)
  {
    EZ_ASSERT_DEBUG(uiIndex < m_uiCount, "Index out of bounds");
    return GetItems()[uiIndex];
  }
} // namespace RPI
