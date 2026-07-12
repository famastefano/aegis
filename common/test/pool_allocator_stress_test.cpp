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
struct Byte_10k_2048S : Params<std::byte, 10'000, 2048>{};

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
    ::testing::Types<Byte_1M_8S, Byte_1M_3S, Byte_1M_1S, Byte_10k_2048S, Short_1M_8S, Short_1M_17S, Short_1M_1S,
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
        else if constexpr (std::is_same_v<T, Byte_10k_2048S>)
            return "Byte_10k_2048S";
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

TYPED_TEST(PoolAllocatorTest, AlternatingSingleSlot)
{
    constexpr auto           iterations      = TypeParam::iterations;
    constexpr auto           slots_per_arena = TypeParam::slots_per_arena;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 0);
    TypeParam               *expectedSlot{};
    for (auto i = 0ull; i < iterations; ++i)
    {
        auto slot = allocator.acquire();
        if (!expectedSlot)
            expectedSlot = slot;

        ASSERT_TRUE(is_aligned_for_type(slot));
        ASSERT_EQ(slot, expectedSlot);
        allocator.release(slot);
        expect_allocator_valid(allocator);
    }
}

TYPED_TEST(PoolAllocatorTest, RandomAcquireRelease)
{
    constexpr auto           iterations      = TypeParam::iterations;
    constexpr auto           slots_per_arena = TypeParam::slots_per_arena;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 0);
    std::vector<TypeParam *> slots;

    std::random_device rd;
    std::mt19937       gen(rd());

    auto pop_random_slot = [&] {
        auto const pos  = std::uniform_int_distribution<std::size_t>{0, slots.size() - 1}(gen);
        auto       slot = slots[pos];
        slots.erase(slots.begin() + pos);
        return slot;
    };

    for (auto i = 0ull; i < iterations; ++i)
    {
        if (slots.empty())
        {
            slots.emplace_back(allocator.acquire());
            expect_aligned_non_null_unique(slots);
        }
        else if (slots.size() % 13 == 0)
        {
            allocator.release(pop_random_slot());
            expect_allocator_valid(allocator);
        }
        else
        {
            if (std::uniform_int_distribution<std::size_t>{0, 1}(gen))
            {
                slots.emplace_back(allocator.acquire());
                expect_aligned_non_null_unique(slots);
            }
            else
            {
                allocator.release(pop_random_slot());
                expect_allocator_valid(allocator);
            }
        }
    }
}

TYPED_TEST(PoolAllocatorTest, AlmostFullAcquireRelease)
{
    constexpr auto           iterations      = TypeParam::iterations;
    constexpr auto           slots_per_arena = TypeParam::slots_per_arena;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 0);
    auto                     slots = acquire_slots(allocator, slots_per_arena - 1);

    std::random_device rd;
    std::mt19937       gen(rd());

    auto pop_random_slot = [&] {
        auto const pos  = std::uniform_int_distribution<std::size_t>{0, slots.size() - 1}(gen);
        auto       slot = slots[pos];
        slots.erase(slots.begin() + pos);
        return slot;
    };

    for (auto i = 0ull; i < iterations; ++i)
    {
        slots.emplace_back(allocator.acquire());
        ASSERT_TRUE(allocator.debug_is_full());
        expect_aligned_non_null_unique(slots);
        allocator.release(pop_random_slot());
        expect_allocator_valid(allocator);
    }
}

TYPED_TEST(PoolAllocatorTest, KeepAllocatorBetween40And60Capacity)
{
    constexpr auto           iterations      = TypeParam::iterations;
    constexpr auto           slots_per_arena = TypeParam::slots_per_arena;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 4);
    std::vector<TypeParam *> slots;
    std::random_device       rd;
    std::mt19937             gen(rd());

    auto pop_random_slot = [&] {
        auto const pos  = std::uniform_int_distribution<std::size_t>{0, slots.size() - 1}(gen);
        auto       slot = slots[pos];
        slots.erase(slots.begin() + pos);
        return slot;
    };

    enum class Phase
    {
        AQUIRE,
        RELEASE
    };
    auto next_phase = [&](Phase curr_phase) {
        if (slots.empty())
            return Phase::AQUIRE;

        float const ratio = allocator.debug_calculate_allocated_ratio();
        if (ratio >= 0.6f)
            return Phase::RELEASE;
        if (ratio < 0.4f)
            return Phase::AQUIRE;
        return curr_phase;
    };
    Phase phase = next_phase(Phase::AQUIRE);

    for (auto i = 0ull; i < iterations; ++i)
    {
        switch (phase)
        {
        case Phase::AQUIRE: {
            slots.emplace_back(allocator.acquire());
            expect_aligned_non_null_unique(slots);
            break;
        }
        case Phase::RELEASE: {
            allocator.release(pop_random_slot());
            expect_allocator_valid(allocator);
            break;
        }
        }
        phase = next_phase(phase);
    }
}

TYPED_TEST(PoolAllocatorTest, MultipleReleaseStrategies)
{
    constexpr auto           iterations      = TypeParam::iterations;
    constexpr auto           slots_per_arena = TypeParam::slots_per_arena;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 4);
    std::vector<TypeParam *> slots;
    std::random_device       rd;
    std::mt19937             gen(rd());

    auto pop_random_slot = [&] {
        auto const pos  = std::uniform_int_distribution<std::size_t>{0, slots.size() - 1}(gen);
        auto       slot = slots[pos];
        slots.erase(slots.begin() + pos);
        return slot;
    };

    for (auto i = 0ull; i < iterations / 4; ++i)
    {
        slots = acquire_slots(allocator, slots_per_arena);
        expect_aligned_non_null_unique(slots);
        std::sort(begin(slots), end(slots));
        release_slots(allocator, slots);
        expect_allocator_valid(allocator);

        slots = acquire_slots(allocator, slots_per_arena);
        expect_aligned_non_null_unique(slots);
        std::sort(begin(slots), end(slots), std::greater{});
        release_slots(allocator, slots);
        expect_allocator_valid(allocator);

        for (auto j = 0ull; j < slots.size(); j += 2)
            allocator.release(slots[j]);
        for (auto k = 1ull; k < slots.size(); k += 2)
            allocator.release(slots[k]);
        expect_allocator_valid(allocator);

        slots = acquire_slots(allocator, slots_per_arena);
        expect_aligned_non_null_unique(slots);
        auto sz = slots.size();
        while (sz--)
            allocator.release(pop_random_slot());
        expect_allocator_valid(allocator);
    }
}

} // namespace
