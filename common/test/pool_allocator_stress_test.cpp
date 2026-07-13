#include "pool_allocator_test_utils.h"

namespace
{
using aegis::allocators::PoolAllocator;
using aegis::allocators::impl::OrderedReleasePolicy;
using aegis::allocators::impl::UnorderedReleasePolicy;

template <typename T, std::size_t TIterations, std::size_t TSlotsPerArena, typename TReleasePolicy> struct Params
{
    using allocator_t = PoolAllocator<T, TReleasePolicy>;
    using allocated_t = T;

    inline static constexpr auto iterations        = TIterations;
    inline static constexpr auto slots_per_arena   = TSlotsPerArena;
    inline static constexpr auto element_size      = sizeof(T);
    inline static constexpr auto element_alignment = alignof(T);

    static constexpr bool is_ordered()
    { return std::is_same_v<OrderedReleasePolicy<T>, TReleasePolicy>; }
};

// clang-format off
struct Byte_1M_8S_O     : Params<uint8_t,  1'000'000,    8, OrderedReleasePolicy  <uint8_t>>{};
struct Byte_1M_8S_U     : Params<uint8_t,  1'000'000,    8, UnorderedReleasePolicy<uint8_t>>{};
struct Byte_1M_3S_O     : Params<uint8_t,  1'000'000,    3, OrderedReleasePolicy  <uint8_t>>{};
struct Byte_1M_3S_U     : Params<uint8_t,  1'000'000,    3, UnorderedReleasePolicy<uint8_t>>{};
struct Byte_1M_1S_O     : Params<uint8_t,  1'000'000,    1, OrderedReleasePolicy  <uint8_t>>{};
struct Byte_1M_1S_U     : Params<uint8_t,  1'000'000,    1, UnorderedReleasePolicy<uint8_t>>{};
struct Byte_10k_512S_O  : Params<uint8_t,     10'000,  512, OrderedReleasePolicy  <uint8_t>>{};
struct Byte_10k_512S_U  : Params<uint8_t,     10'000,  512, UnorderedReleasePolicy<uint8_t>>{};

struct Short_1M_8S_O    : Params<uint16_t, 1'000'000,    8, OrderedReleasePolicy  <uint16_t>>{};
struct Short_1M_8S_U    : Params<uint16_t, 1'000'000,    8, UnorderedReleasePolicy<uint16_t>>{};
struct Short_1M_17S_O   : Params<uint16_t, 1'000'000,   17, OrderedReleasePolicy  <uint16_t>>{};
struct Short_1M_17S_U   : Params<uint16_t, 1'000'000,   17, UnorderedReleasePolicy<uint16_t>>{};
struct Short_1M_1S_O    : Params<uint16_t, 1'000'000,    1, OrderedReleasePolicy  <uint16_t>>{};
struct Short_1M_1S_U    : Params<uint16_t, 1'000'000,    1, UnorderedReleasePolicy<uint16_t>>{};

struct Int_1M_8S_O      : Params<uint32_t, 1'000'000,    8, OrderedReleasePolicy  <uint32_t>>{};
struct Int_1M_8S_U      : Params<uint32_t, 1'000'000,    8, UnorderedReleasePolicy<uint32_t>>{};
struct Int_1M_67S_O     : Params<uint32_t, 1'000'000,   67, OrderedReleasePolicy  <uint32_t>>{};
struct Int_1M_67S_U     : Params<uint32_t, 1'000'000,   67, UnorderedReleasePolicy<uint32_t>>{};
struct Int_1M_1S_O      : Params<uint32_t, 1'000'000,    1, OrderedReleasePolicy  <uint32_t>>{};
struct Int_1M_1S_U      : Params<uint32_t, 1'000'000,    1, UnorderedReleasePolicy<uint32_t>>{};

struct Long_1M_8S_O     : Params<uint64_t, 1'000'000,    8, OrderedReleasePolicy  <uint64_t>>{};
struct Long_1M_8S_U     : Params<uint64_t, 1'000'000,    8, UnorderedReleasePolicy<uint64_t>>{};
struct Long_1M_13S_O    : Params<uint64_t, 1'000'000,   13, OrderedReleasePolicy  <uint64_t>>{};
struct Long_1M_13S_U    : Params<uint64_t, 1'000'000,   13, UnorderedReleasePolicy<uint64_t>>{};
struct Long_1M_1S_O     : Params<uint64_t, 1'000'000,    1, OrderedReleasePolicy  <uint64_t>>{};
struct Long_1M_1S_U     : Params<uint64_t, 1'000'000,    1, UnorderedReleasePolicy<uint64_t>>{};

struct BigData { std::byte _[1337]; };
struct Big_1K_8S_O      : Params<BigData,      1'000,    8, OrderedReleasePolicy  <BigData>> {};
struct Big_1K_8S_U      : Params<BigData,      1'000,    8, UnorderedReleasePolicy<BigData>> {};
struct Big_1K_41S_O     : Params<BigData,      1'000,   41, OrderedReleasePolicy  <BigData>> {};
struct Big_1K_41S_U     : Params<BigData,      1'000,   41, UnorderedReleasePolicy<BigData>> {};
struct Big_1K_1S_O      : Params<BigData,      1'000,    1, OrderedReleasePolicy  <BigData>> {};
struct Big_1K_1S_U      : Params<BigData,      1'000,    1, UnorderedReleasePolicy<BigData>> {};

struct alignas(64) BigCache { std::byte _[64]; };
struct Cache_1K_8S_O    : Params<BigCache,     1'000,    8, OrderedReleasePolicy  <BigCache>> {};
struct Cache_1K_8S_U    : Params<BigCache,     1'000,    8, UnorderedReleasePolicy<BigCache>> {};
struct Cache_1K_37S_O   : Params<BigCache,     1'000,   37, OrderedReleasePolicy  <BigCache>> {};
struct Cache_1K_37S_U   : Params<BigCache,     1'000,   37, UnorderedReleasePolicy<BigCache>> {};
struct Cache_1K_1S_O    : Params<BigCache,     1'000,    1, OrderedReleasePolicy  <BigCache>> {};
struct Cache_1K_1S_U    : Params<BigCache,     1'000,    1, UnorderedReleasePolicy<BigCache>> {};

struct alignas(4096) PageAlignData { std::byte _[32]; };
struct Page_1K_8S_O     : Params<PageAlignData, 1'000,   8, OrderedReleasePolicy  <PageAlignData>> {};
struct Page_1K_8S_U     : Params<PageAlignData, 1'000,   8, UnorderedReleasePolicy<PageAlignData>> {};
struct Page_1K_5S_O     : Params<PageAlignData, 1'000,   5, OrderedReleasePolicy  <PageAlignData>> {};
struct Page_1K_5S_U     : Params<PageAlignData, 1'000,   5, UnorderedReleasePolicy<PageAlignData>> {};
struct Page_1K_1S_O     : Params<PageAlignData, 1'000,   1, OrderedReleasePolicy  <PageAlignData>> {};
struct Page_1K_1S_U     : Params<PageAlignData, 1'000,   1, UnorderedReleasePolicy<PageAlignData>> {};

// clang-format on

template <typename> constexpr bool dependent_false_v = false;

template <typename T> class PoolAllocatorTest : public ::testing::Test
{
};

using PoolAllocatorTestTypes =
    ::testing::Types<Byte_1M_8S_O, Byte_1M_8S_U, Byte_1M_3S_O, Byte_1M_3S_U, Byte_1M_1S_O, Byte_1M_1S_U,
                     Byte_10k_512S_O, Byte_10k_512S_U, Short_1M_8S_O, Short_1M_8S_U, Short_1M_17S_O, Short_1M_17S_U,
                     Short_1M_1S_O, Short_1M_1S_U, Int_1M_8S_O, Int_1M_8S_U, Int_1M_67S_O, Int_1M_67S_U, Int_1M_1S_O,
                     Int_1M_1S_U, Long_1M_8S_O, Long_1M_8S_U, Long_1M_13S_O, Long_1M_13S_U, Long_1M_1S_O, Long_1M_1S_U,
                     Big_1K_8S_O, Big_1K_8S_U, Big_1K_41S_O, Big_1K_41S_U, Big_1K_1S_O, Big_1K_1S_U, Cache_1K_8S_O,
                     Cache_1K_8S_U, Cache_1K_37S_O, Cache_1K_37S_U, Cache_1K_1S_O, Cache_1K_1S_U, Page_1K_8S_O,
                     Page_1K_8S_U, Page_1K_5S_O, Page_1K_5S_U, Page_1K_1S_O, Page_1K_1S_U>;

struct PoolAllocatorTypeNames
{
    template <typename T> static std::string GetName(int)
    {
        // clang-format off
#define AEGIS_TEST_IF_NAME(Name)        if constexpr (std::is_same_v<T, Name>) return #Name;
#define AEGIS_TEST_ELIF_NAME(Name) else if constexpr (std::is_same_v<T, Name>) return #Name;
#define AEGIS_TEST_ELSE(Name)      else { static_assert(dependent_false_v<T>, "Unexpected PoolAllocator typed-test parameter"); return #Name; }
        // clang-format on

        AEGIS_TEST_IF_NAME(Byte_1M_8S_O)
        AEGIS_TEST_ELIF_NAME(Byte_1M_8S_U)
        AEGIS_TEST_ELIF_NAME(Byte_1M_3S_O)
        AEGIS_TEST_ELIF_NAME(Byte_1M_3S_U)
        AEGIS_TEST_ELIF_NAME(Byte_1M_1S_O)
        AEGIS_TEST_ELIF_NAME(Byte_1M_1S_U)
        AEGIS_TEST_ELIF_NAME(Byte_10k_512S_O)
        AEGIS_TEST_ELIF_NAME(Byte_10k_512S_U)
        AEGIS_TEST_ELIF_NAME(Short_1M_8S_O)
        AEGIS_TEST_ELIF_NAME(Short_1M_8S_U)
        AEGIS_TEST_ELIF_NAME(Short_1M_17S_O)
        AEGIS_TEST_ELIF_NAME(Short_1M_17S_U)
        AEGIS_TEST_ELIF_NAME(Short_1M_1S_O)
        AEGIS_TEST_ELIF_NAME(Short_1M_1S_U)
        AEGIS_TEST_ELIF_NAME(Int_1M_8S_O)
        AEGIS_TEST_ELIF_NAME(Int_1M_8S_U)
        AEGIS_TEST_ELIF_NAME(Int_1M_67S_O)
        AEGIS_TEST_ELIF_NAME(Int_1M_67S_U)
        AEGIS_TEST_ELIF_NAME(Int_1M_1S_O)
        AEGIS_TEST_ELIF_NAME(Int_1M_1S_U)
        AEGIS_TEST_ELIF_NAME(Long_1M_8S_O)
        AEGIS_TEST_ELIF_NAME(Long_1M_8S_U)
        AEGIS_TEST_ELIF_NAME(Long_1M_13S_O)
        AEGIS_TEST_ELIF_NAME(Long_1M_13S_U)
        AEGIS_TEST_ELIF_NAME(Long_1M_1S_O)
        AEGIS_TEST_ELIF_NAME(Long_1M_1S_U)
        AEGIS_TEST_ELIF_NAME(Big_1K_8S_O)
        AEGIS_TEST_ELIF_NAME(Big_1K_8S_U)
        AEGIS_TEST_ELIF_NAME(Big_1K_41S_O)
        AEGIS_TEST_ELIF_NAME(Big_1K_41S_U)
        AEGIS_TEST_ELIF_NAME(Big_1K_1S_O)
        AEGIS_TEST_ELIF_NAME(Big_1K_1S_U)
        AEGIS_TEST_ELIF_NAME(Cache_1K_8S_O)
        AEGIS_TEST_ELIF_NAME(Cache_1K_8S_U)
        AEGIS_TEST_ELIF_NAME(Cache_1K_37S_O)
        AEGIS_TEST_ELIF_NAME(Cache_1K_37S_U)
        AEGIS_TEST_ELIF_NAME(Cache_1K_1S_O)
        AEGIS_TEST_ELIF_NAME(Cache_1K_1S_U)
        AEGIS_TEST_ELIF_NAME(Page_1K_8S_O)
        AEGIS_TEST_ELIF_NAME(Page_1K_8S_U)
        AEGIS_TEST_ELIF_NAME(Page_1K_5S_O)
        AEGIS_TEST_ELIF_NAME(Page_1K_5S_U)
        AEGIS_TEST_ELIF_NAME(Page_1K_1S_O)
        AEGIS_TEST_ELIF_NAME(Page_1K_1S_U)
        AEGIS_TEST_ELSE(Unknown)

#undef AEGIS_TEST_IF_NAME
#undef AEGIS_TEST_ELIF_NAME
#undef AEGIS_TEST_ELSE
    }
};

TYPED_TEST_SUITE(PoolAllocatorTest, PoolAllocatorTestTypes, PoolAllocatorTypeNames);

TYPED_TEST(PoolAllocatorTest, FillDrainWithPeriodicReverseRelease)
{
    constexpr auto                                 iterations      = TypeParam::iterations;
    constexpr auto                                 slots_per_arena = TypeParam::slots_per_arena;
    typename TypeParam::allocator_t                allocator(slots_per_arena, 0);
    std::vector<typename TypeParam::allocated_t *> prev_slots;
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
    constexpr auto                   iterations      = TypeParam::iterations;
    constexpr auto                   slots_per_arena = TypeParam::slots_per_arena;
    typename TypeParam::allocator_t  allocator(slots_per_arena, 0);
    typename TypeParam::allocated_t *expectedSlot{};
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
    constexpr auto                                 iterations      = TypeParam::iterations;
    constexpr auto                                 slots_per_arena = TypeParam::slots_per_arena;
    typename TypeParam::allocator_t                allocator(slots_per_arena, 0);
    std::vector<typename TypeParam::allocated_t *> slots;

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
    constexpr auto                  iterations      = TypeParam::iterations;
    constexpr auto                  slots_per_arena = TypeParam::slots_per_arena;
    typename TypeParam::allocator_t allocator(slots_per_arena, 0);
    auto                            slots = acquire_slots(allocator, slots_per_arena - 1);

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
    constexpr auto                                 iterations      = TypeParam::iterations;
    constexpr auto                                 slots_per_arena = TypeParam::slots_per_arena;
    typename TypeParam::allocator_t                allocator(slots_per_arena, 4);
    std::vector<typename TypeParam::allocated_t *> slots;
    std::random_device                             rd;
    std::mt19937                                   gen(rd());

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
    constexpr auto                                 iterations      = TypeParam::iterations;
    constexpr auto                                 slots_per_arena = TypeParam::slots_per_arena;
    typename TypeParam::allocator_t                allocator(slots_per_arena, 4);
    std::vector<typename TypeParam::allocated_t *> slots;
    std::random_device                             rd;
    std::mt19937                                   gen(rd());

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

        slots = acquire_slots(allocator, slots_per_arena);
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
