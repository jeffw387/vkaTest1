#include "monotonic_allocator.hpp"
#include <catch2/catch.hpp>
#include <vector>

TEST_CASE("Create memory resource of size 1024") {
  REQUIRE_NOTHROW([]{ monotonic_memory memoryResource{1024}; }());
}

TEST_CASE("Create an allocator from a memory resource") {
  monotonic_memory memoryResource{1024};
  REQUIRE_NOTHROW([](auto mem){ monotonic_allocator<int> monotonic{mem}; }(&memoryResource));
}

TEST_CASE("Allocate an int from an allocator with sizeof(int)") {
  monotonic_memory memoryResource{sizeof(int)};
  monotonic_allocator<int> monotonic{&memoryResource};
  REQUIRE_NOTHROW([&]{auto ptr = monotonic.allocate(1);}());
  REQUIRE_THROWS([&]{auto ptr = monotonic.allocate(1);}());
}

TEST_CASE("Allocate two ints from an allocator with sizeof(int) * 2") {
  monotonic_memory memoryResource{sizeof(int) * 2};
  monotonic_allocator<int> monotonic{&memoryResource};
  REQUIRE_NOTHROW([&]{auto ptr = monotonic.allocate(2);}());
  REQUIRE_THROWS([&]{auto ptr = monotonic.allocate(1);}());
}

TEST_CASE("Allocate two ints from an allocator with size 8 + 1") {
  monotonic_memory memoryResource{8 + 1};
  monotonic_allocator<int32_t> monotonic{&memoryResource};
  REQUIRE_NOTHROW([&]{auto ptr = monotonic.allocate(2);}());
  REQUIRE_THROWS([&]{auto ptr = monotonic.allocate(1);}());
}

TEST_CASE("Allocate two ints from an allocator with size 8 + 2") {
  monotonic_memory memoryResource{8 + 2};
  monotonic_allocator<int32_t> monotonic{&memoryResource};
  REQUIRE_NOTHROW([&]{auto ptr = monotonic.allocate(2);}());
  REQUIRE_THROWS([&]{auto ptr = monotonic.allocate(1);}());
}

TEST_CASE("Allocate two ints from an allocator with size 8 + 3") {
  monotonic_memory memoryResource{8 + 3};
  monotonic_allocator<int32_t> monotonic{&memoryResource};
  REQUIRE_NOTHROW([&]{auto ptr = monotonic.allocate(2);}());
  REQUIRE_THROWS([&]{auto ptr = monotonic.allocate(1);}());
}

struct alignas(128) test_aligned_128 {};
TEST_CASE("Allocate an empty alignas(128) struct from a pool sized 128") {
  monotonic_memory memoryResource{128};
  monotonic_allocator<test_aligned_128> monotonic{&memoryResource};
  REQUIRE_NOTHROW([&]{auto ptr = monotonic.allocate(1);}());
  REQUIRE_THROWS([&]{auto ptr = monotonic.allocate(1);}());
}

TEST_CASE("Exhaust the allocator memory, reset, allocate successfully") {
  monotonic_memory memoryResource{128};
  monotonic_allocator<test_aligned_128> monotonic{&memoryResource};
  auto ptr = monotonic.allocate(1);
  monotonic.reset();
  REQUIRE_NOTHROW([&]{auto ptr2 = monotonic.allocate(1);}());
}

TEST_CASE("Allocate a struct aligned to 4 bytes, then one aligned to 8 bytes. Size of pool should be 16. Next allocation of any size should fail.") {
  monotonic_memory memoryResource{16};
  monotonic_allocator<int32_t> alloc4{&memoryResource};
  monotonic_allocator<int64_t> alloc8{&memoryResource};
  auto ptr0 = alloc4.allocate(1);
  auto ptr1 = alloc8.allocate(1);
  REQUIRE((reinterpret_cast<size_t>(ptr1) % 8) == 0);
  REQUIRE_THROWS([&]{ auto ptr2 = alloc4.allocate(1); }());
}

TEST_CASE("Two allocators of the same type with the same memory resource should compare equal") {
  monotonic_memory memoryResource{16};
  monotonic_allocator<int32_t> alloc0{&memoryResource};
  monotonic_allocator<int32_t> alloc1{&memoryResource};
  REQUIRE(alloc0 == alloc1);
}

TEST_CASE("Two allocators of the same type referencing different memory resources should compare not equal") {
  monotonic_memory memoryResource0{16};
  monotonic_memory memoryResource1{16};
  monotonic_allocator<int32_t> alloc0{&memoryResource0};
  monotonic_allocator<int32_t> alloc1{&memoryResource1};
  REQUIRE(alloc0 != alloc1);
}

TEST_CASE("Allocate a vector's data from a monotonic allocator") {
  monotonic_memory memoryResource{sizeof(int32_t) * 8};
  monotonic_allocator<int32_t> alloc{&memoryResource};
  std::vector<int32_t, monotonic_allocator<int32_t>> intVector{alloc};
  REQUIRE_NOTHROW(intVector.reserve(8));
  for (int i{}; i < 8; ++i) {
    REQUIRE_NOTHROW(intVector.push_back(i));
  }
  REQUIRE_THROWS(intVector.push_back(8));
}