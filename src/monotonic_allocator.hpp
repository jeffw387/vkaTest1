#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

struct monotonic_memory {
  monotonic_memory(size_t size) : m_size(size) {
    m_memory = malloc(size);
    m_freePtr = m_memory;
  }

  monotonic_memory(const monotonic_memory&) = delete;
  monotonic_memory(monotonic_memory&&) = default;
  monotonic_memory& operator=(const monotonic_memory&) = delete;
  monotonic_memory& operator=(monotonic_memory&&) = default;

  ~monotonic_memory() {
    free(m_memory);
  }

  template <typename T>
  T* allocate(size_t n) {
    constexpr auto alignment = alignof(T);
    size_t alignedSize = alignment * n;
    m_freePtr = next_aligned(alignment);
    if (!can_allocate(alignedSize)) {
      throw std::bad_alloc{};
    }
    auto result = m_freePtr;
    auto newFree = reinterpret_cast<size_t>(m_freePtr) + alignedSize;
    m_freePtr = reinterpret_cast<void*>(newFree);
    return reinterpret_cast<T*>(result);
  }

  void reset() {
    m_freePtr = m_memory;
  }
private:
  size_t m_size{};
  void* m_memory{};
  void* m_freePtr{};

  size_t distance_base_to_free() {
    return reinterpret_cast<size_t>(m_freePtr) 
      - reinterpret_cast<size_t>(m_memory);
  }

  void* next_aligned(std::size_t requiredAlignment) {
    size_t dist = distance_base_to_free();
    size_t nextAligned = reinterpret_cast<size_t>(m_freePtr) + (dist % requiredAlignment);
    return reinterpret_cast<void*>(std::min(nextAligned, 
      reinterpret_cast<size_t>(end_pointer())));
  }

  bool can_allocate(size_t requiredSize) {
    return (distance_base_to_free() + requiredSize) <= m_size;
  }

  void* end_pointer() {
    return reinterpret_cast<void*>(reinterpret_cast<size_t>(m_memory) + m_size);
  }
};

template <typename T>
struct monotonic_allocator;

template <typename T>
void swap(monotonic_allocator<T>, monotonic_allocator<T>) noexcept;

template <typename T>
bool operator!=(monotonic_allocator<T>, monotonic_allocator<T>) noexcept;

template <typename T>
bool operator==(monotonic_allocator<T>, monotonic_allocator<T>) noexcept;

template <typename T>
struct monotonic_allocator {
  using value_type = T;

  monotonic_allocator(monotonic_memory* memoryResource)
  : m_memoryResource(memoryResource) {
  }

  T* allocate(size_t n) {
    return m_memoryResource->allocate<T>(n);
  }

  void deallocate(T*, size_t n) {};

  void reset() {
    m_memoryResource->reset();
  }
  template <typename U>
  friend void swap(monotonic_allocator<U>, monotonic_allocator<U>) noexcept;
  template <typename U>
  friend bool operator!=(monotonic_allocator<U>,monotonic_allocator<U>) noexcept;
  template <typename U>
  friend bool operator==(monotonic_allocator<U>,monotonic_allocator<U>) noexcept;

private:
  monotonic_memory* m_memoryResource{};
};

template <typename T>
void swap(monotonic_allocator<T> lhs, monotonic_allocator<T> rhs) noexcept {
  std::swap(lhs.m_memoryResource, rhs.m_memoryResource);
}

template <typename T>
bool operator!=(monotonic_allocator<T> lhs, monotonic_allocator<T> rhs) noexcept {
  return lhs.m_memoryResource != rhs.m_memoryResource;
}

template <typename T>
bool operator==(monotonic_allocator<T> lhs, monotonic_allocator<T> rhs) noexcept {
  return !(lhs != rhs);
}