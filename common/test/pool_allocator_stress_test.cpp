#include "pool_allocator_test_utils.h"

namespace
{
using aegis::allocators::PoolAllocator;

template <typename T, std::size_t TIterations, std::size_t TSlotsPerArena> struct Params
{
    inline static constexpr auto iterations        = TIterations;
    inline static constexpr auto slots_per_arena   = TSlotsPerArena;
    inline static constexpr auto element_size      = sizeof(T);
    inline static constexpr auto element_alignment = alignof(T);

    T _;
};

// clang-format off
struct Byte_1M_8S : Params<std::byte, 1'000'000, 8>{};
struct Byte_1M_3S : Params<std::byte, 1'000'000, 3>{};
struct Byte_1M_1S : Params<std::byte, 1'000'000, 1>{};
struct Byte_10k_4096S : Params<std::byte, 10'000, 4096>{};

struct Short_1M_8S : Params<std::uint16_t, 1'000'000, 8>{};
struct Short_1M_17S : Params<std::uint16_t, 1'000'000, 17>{};
struct Short_1M_1S : Params<std::uint16_t, 1'000'000, 1>{};

struct Int_1M_8S : Params<std::uint32_t, 1'000'000, 8>{};
struct Int_1M_67S : Params<std::uint32_t, 1'000'000, 67>{};
struct Int_1M_1S : Params<std::uint32_t, 1'000'000, 1>{};

struct Long_1M_8S : Params<std::uint64_t, 1'000'000, 8>{};
struct Long_1M_13S : Params<std::uint64_t, 1'000'000, 13>{};
struct Long_1M_1S : Params<std::uint64_t, 1'000'000, 1>{};

struct BigData { std::byte _[1337]; };
struct Big_1K_8S : Params<BigData, 1'000, 8> {};
struct Big_1K_41S : Params<BigData, 1'000, 41> {};
struct Big_1K_1S : Params<BigData, 1'000, 1> {};

struct alignas(64) CacheAlignData { std::byte _[64]; };
struct Cache_1K_8S : Params<CacheAlignData, 1'000, 8> {};
struct Cache_1K_37S : Params<CacheAlignData, 1'000, 37> {};
struct Cache_1K_1S : Params<CacheAlignData, 1'000, 1> {};

struct alignas(4096) PageAlignData { std::byte _[32]; };
struct Page_1K_8S : Params<PageAlignData, 1'000, 8> {};
struct Page_1K_5S : Params<PageAlignData, 1'000, 5> {};
struct Page_1K_1S : Params<PageAlignData, 1'000, 1> {};

// clang-format on

template <typename> constexpr bool dependent_false_v = false;

template <typename T> class PoolAllocatorTest : public ::testing::Test
{
};

 using PoolAllocatorTestTypes =
     ::testing::Types<Byte_1M_8S, Byte_1M_3S, Byte_1M_1S, Byte_10k_4096S, Short_1M_8S, Short_1M_17S, Short_1M_1S,
                      Int_1M_8S, Int_1M_67S, Int_1M_1S, Long_1M_8S, Long_1M_13S, Long_1M_1S, Big_1K_8S, Big_1K_41S,
                      Big_1K_1S, Cache_1K_8S, Cache_1K_37S, Cache_1K_1S, Page_1K_8S, Page_1K_5S, Page_1K_1S>;

struct PoolAllocatorTypeNames
{
    template <typename T> static std::string GetName(int)
    {
        if constexpr (std::is_same_v<T, Byte_1M_8S>)
            return "Byte_1M_8S";
        else if constexpr (std::is_same_v<T, Byte_1M_3S>)
            return "Byte_1M_3S";
        else if constexpr (std::is_same_v<T, Byte_1M_1S>)
            return "Byte_1M_1S";
        else if constexpr (std::is_same_v<T, Byte_10k_4096S>)
            return "Byte_10k_4096S";
        else if constexpr (std::is_same_v<T, Short_1M_8S>)
            return "Short_1M_8S";
        else if constexpr (std::is_same_v<T, Short_1M_17S>)
            return "Short_1M_17S";
        else if constexpr (std::is_same_v<T, Short_1M_1S>)
            return "Short_1M_1S";
        else if constexpr (std::is_same_v<T, Int_1M_8S>)
            return "Int_1M_8S";
        else if constexpr (std::is_same_v<T, Int_1M_67S>)
            return "Int_1M_67S";
        else if constexpr (std::is_same_v<T, Int_1M_1S>)
            return "Int_1M_1S";
        else if constexpr (std::is_same_v<T, Long_1M_8S>)
            return "Long_1M_8S";
        else if constexpr (std::is_same_v<T, Long_1M_13S>)
            return "Long_1M_13S";
        else if constexpr (std::is_same_v<T, Long_1M_1S>)
            return "Long_1M_1S";
        else if constexpr (std::is_same_v<T, Big_1K_8S>)
            return "Big_1K_8S";
        else if constexpr (std::is_same_v<T, Big_1K_41S>)
            return "Big_1K_41S";
        else if constexpr (std::is_same_v<T, Big_1K_1S>)
            return "Big_1K_1S";
        else if constexpr (std::is_same_v<T, Cache_1K_8S>)
            return "Cache_1K_8S";
        else if constexpr (std::is_same_v<T, Cache_1K_37S>)
            return "Cache_1K_37S";
        else if constexpr (std::is_same_v<T, Cache_1K_1S>)
            return "Cache_1K_1S";
        else if constexpr (std::is_same_v<T, Page_1K_8S>)
            return "Page_1K_8S";
        else if constexpr (std::is_same_v<T, Page_1K_5S>)
            return "Page_1K_5S";
        else if constexpr (std::is_same_v<T, Page_1K_1S>)
            return "Page_1K_1S";
        else
        {
            static_assert(dependent_false_v<T>, "Unexpected PoolAllocator typed-test parameter");
            return {};
        }
    }
};

TYPED_TEST_SUITE(PoolAllocatorTest, PoolAllocatorTestTypes, PoolAllocatorTypeNames);

TYPED_TEST(PoolAllocatorTest, FillDrainWithPeriodicReverseRelease)
{
    constexpr auto           iterations      = TypeParam::iterations;
    constexpr auto           slots_per_arena = TypeParam::slots_per_arena;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 0);
    std::vector<TypeParam *> prev_slots;
    for (auto i = 0ull; i < iterations; ++i)
    {
        auto slots = acquire_slots(allocator, slots_per_arena);
        if (prev_slots.empty())
            prev_slots = slots;

        expect_same_pointer_set(slots, prev_slots);

        if (i % 1'000 == 0)
            release_slots_reversed(allocator, slots);
        else
            release_slots(allocator, slots);
        expect_allocator_valid(allocator);
    }
    expect_aligned_non_null_unique(prev_slots);
}

/*
1. **Fill/drain**

Allocate until full, release all, repeat many times.

```text
repeat 1'000'000 times:
    allocate N slots
    assert next acquire fails
    release all N slots
```

Catches exhaustion bugs, reset bugs, and free-count corruption.

2. **Alternating single slot**

```text
repeat many times:
    p = acquire()
    release(p)
```

Catches freelist head bugs and use-after-release issues.

3. **Random churn**

```text
live = []

repeat 10'000'000 times:
    if live.empty(): acquire
    else if live.size() == N: release random live pointer
    else randomly acquire or release

    assert invariants
```

This is the most useful general-purpose stress test.

4. **High-watermark churn**

Keep the pool almost full.

```text
allocate N - 1 slots

repeat many times:
    p = acquire()
    assert pool is full
    release(random live pointer)
```

Catches bugs that only appear near capacity.

5. **Sparse churn**

Keep only a few live slots.

```text
repeat many times:
    allocate 1..4 slots
    release them in random order
```

Useful for freelist reuse behavior and bitmap search edge cases.

6. **Release-order permutations**

For small `N`, allocate all slots, then release in different orders:

```text
release ascending
release descending
release even slots first
release odd slots first
release random permutation
```

Then allocate all again and verify uniqueness/alignment.

7. **Type/alignment stress**

Run the same stress suite for:

```cpp
char
std::uint16_t
std::uint64_t
struct alignas(16) A { char x; };
struct alignas(64) B { char x; };
struct Big { std::byte data[257]; };
```

This catches stride and alignment errors.

8. **Multi-arena stress**

If your allocator supports multiple arenas or growth:

```text
allocate more than one arena worth
release mixed pointers from different arenas
allocate again
verify pointers belong to valid arenas
```

9. **Threaded stress**, only if allocator is meant to be thread-safe.

```text
M threads:
    each repeatedly acquire/release
    optionally write thread id into acquired slot
```

Run with ThreadSanitizer when using Clang. Do not run ASan and TSan in the same binary.

For every test, keep an external oracle:

```text
live set
free count
all returned pointers
expected slot indices
```

And after each operation check:

```text
pointer is inside arena
pointer is aligned
pointer is not already live
live_count <= capacity
released pointer was live
no duplicate live pointers
allocator debug_validate() passes
```

*/
} // namespace
