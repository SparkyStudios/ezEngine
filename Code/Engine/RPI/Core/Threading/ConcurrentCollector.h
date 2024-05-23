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

#pragma once

#include <RPI/RPIDLL.h>

namespace RPI
{
  /// \brief A collector that allows for concurrent adding of items,
  /// as well as non-thread-safe clearing and accessing of the underlying collection.
  template <typename T, typename TAllocatorWrapper = ezDefaultAllocatorWrapper>
  class spConcurrentCollector
  {
  public:
    /// \brief Constructor.
    /// \param uiInitialCapacity The initial capacity of the collection.
    explicit spConcurrentCollector(ezUInt32 uiInitialCapacity = 16);

    virtual ~spConcurrentCollector();

    /// \brief Gets a contiguous range of items. It is valid to call this method
    /// only after the collection is closed.
    EZ_NODISCARD EZ_ALWAYS_INLINE const ezHybridArray<T, 16, TAllocatorWrapper>& GetItems() const
    {
      EZ_ASSERT_DEBUG(m_pHead == m_pTail, "The collection is not yet closed.");
      return m_pHead->m_Items;
    }

    /// \brief Gets a contiguous range of items. It is valid to call this method
    /// only after the collection is closed.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezHybridArray<T, 16, TAllocatorWrapper>& GetItems()
    {
      EZ_ASSERT_DEBUG(m_pHead == m_pTail, "The collection is not yet closed.");
      return m_pHead->m_Items;
    }

    /// \brief Gets the number of items in the collection.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetCount() const { return m_uiCount; }

    /// \brief Checks whether the collection is empty.
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsEmpty() const { return m_uiCount == 0; }

    /// \brief Consolidates all items into a single contiguous array.
    /// \note This method should be called before to access the collection.
    void Close();

    /// \brief Adds an item to the collection.
    ezInt32 Add(const T& item);

    /// \brief Adds a range of items to the collection.
    void AddRange(const ezArrayPtr<T>& items);

    /// \brief Clears the collection.
    void Clear();

    /// \brief Returns the element at the given index. Does bounds checks in debug builds.
    const T& operator[](ezUInt32 uiIndex) const;

    /// \brief Returns the element at the given index. Does bounds checks in debug builds.
    T& operator[](ezUInt32 uiIndex);

    using const_iterator = const T*;
    using const_reverse_iterator = const_reverse_pointer_iterator<T>;
    using iterator = T*;
    using reverse_iterator = reverse_pointer_iterator<T>;

  private:
    class Segment
    {
    public:
      explicit Segment(ezUInt32 uiInitialCapacity)
      {
        m_Items.SetCountUninitialized(uiInitialCapacity);
      }

      ezHybridArray<T, 16, TAllocatorWrapper> m_Items;
      ezUInt32 m_uiOffset{0};
      Segment* m_pNext{nullptr};
      Segment* m_pPrev{nullptr};
    };

    ezMutex m_ResizeMutex;

    Segment* m_pHead{nullptr};
    Segment* m_pTail{nullptr};
    ezAtomicInteger32 m_uiCount{0};
  };

  template <typename T>
  typename spConcurrentCollector<T>::iterator begin(spConcurrentCollector<T>& ref_container)
  {
    return ref_container.GetItems().GetData();
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_iterator begin(const spConcurrentCollector<T>& container)
  {
    return container.GetItems().GetData();
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_iterator cbegin(const spConcurrentCollector<T>& container)
  {
    return container.GetItems().GetData();
  }

  template <typename T>
  typename spConcurrentCollector<T>::reverse_iterator rbegin(spConcurrentCollector<T>& ref_container)
  {
    return typename spConcurrentCollector<T>::reverse_iterator(ref_container.GetItems().GetData() + ref_container.GetCount() - 1);
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_reverse_iterator rbegin(const spConcurrentCollector<T>& container)
  {
    return typename spConcurrentCollector<T>::const_reverse_iterator(container.GetItems().GetData() + container.GetCount() - 1);
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_reverse_iterator crbegin(const spConcurrentCollector<T>& container)
  {
    return typename spConcurrentCollector<T>::const_reverse_iterator(container.GetItems().GetData() + container.GetCount() - 1);
  }

  template <typename T>
  typename spConcurrentCollector<T>::iterator end(spConcurrentCollector<T>& ref_container)
  {
    return ref_container.GetItems().GetData() + ref_container.GetCount();
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_iterator end(const spConcurrentCollector<T>& container)
  {
    return container.GetItems().GetData() + container.GetCount();
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_iterator cend(const spConcurrentCollector<T>& container)
  {
    return container.GetItems().GetData() + container.GetCount();
  }

  template <typename T>
  typename spConcurrentCollector<T>::reverse_iterator rend(spConcurrentCollector<T>& ref_container)
  {
    return typename spConcurrentCollector<T>::reverse_iterator(ref_container.GetItems().GetData() - 1);
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_reverse_iterator rend(const spConcurrentCollector<T>& container)
  {
    return typename spConcurrentCollector<T>::const_reverse_iterator(container.GetItems().GetData() - 1);
  }

  template <typename T>
  typename spConcurrentCollector<T>::const_reverse_iterator crend(const spConcurrentCollector<T>& container)
  {
    return typename spConcurrentCollector<T>::const_reverse_iterator(container.GetItems().GetData() - 1);
  }
} // namespace RPI

#include <RPI/Implementation/Core/Threading/ConcurrentCollector_inl.h>
