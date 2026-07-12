#include "pool_allocator_test_utils.h"

namespace
{
template <typename T> struct OrderedAllocatorParams
{
    using allocator_t = typename aegis::allocators::OrderedPoolAllocator<T>;

    static constexpr bool is_ordered()
    { return true; }
};

template <typename T> struct UnorderedAllocatorParams
{
    using allocator_t = typename aegis::allocators::UnorderedPoolAllocator<T>;

    static constexpr bool is_ordered()
    { return false; }
};

#define AEGIS_TEST_TYPE(Type, SzExpr, Params) \
    struct Type : Params<Type>                \
    {                                         \
        std::byte bytes[SzExpr];              \
    }

#define AEGIS_TEST_TYPE_ALIGNED(Type, SzExpr, Params, Align) \
    struct alignas(Align) Type : Params<Type>                \
    {                                                        \
        std::byte bytes[SzExpr];                             \
    }

#define AEGIS_TEST_RAW_TYPE(Type, Storage, Params) \
    struct Type : Params<Type>                     \
    {                                              \
        Storage _;                                 \
    }

#define AEGIS_TEST_RAW_TYPE_ALIGNED(Type, Storage, Params, Align) \
    struct alignas(Align) Type : Params<Type>                     \
    {                                                             \
        alignas(Align) Storage _;                                 \
    }

#define AEGIS_CONCAT(A, B) A##B

// clang-format off
#define AEGIS_TEST_TYPE_O(Type, SzExpr)          AEGIS_TEST_TYPE(AEGIS_CONCAT(Type, _O), SzExpr, OrderedAllocatorParams)
#define AEGIS_TEST_TYPE_O_A(Type, SzExpr, Align) AEGIS_TEST_TYPE_ALIGNED(AEGIS_CONCAT(Type, _O), SzExpr, OrderedAllocatorParams, Align)
#define AEGIS_TEST_TYPE_U(Type, SzExpr)          AEGIS_TEST_TYPE(AEGIS_CONCAT(Type, _U), SzExpr, UnorderedAllocatorParams)
#define AEGIS_TEST_TYPE_U_A(Type, SzExpr, Align) AEGIS_TEST_TYPE_ALIGNED(AEGIS_CONCAT(Type, _U), SzExpr, UnorderedAllocatorParams, Align)

#define AEGIS_TEST_RAW_TYPE_O(Type, Storage)          AEGIS_TEST_RAW_TYPE(AEGIS_CONCAT(Type, _O), Storage, OrderedAllocatorParams)
#define AEGIS_TEST_RAW_TYPE_O_A(Type, Storage, Align) AEGIS_TEST_RAW_TYPE_ALIGNED(AEGIS_CONCAT(Type, _O), Storage, OrderedAllocatorParams, Align)
#define AEGIS_TEST_RAW_TYPE_U(Type, Storage)          AEGIS_TEST_RAW_TYPE(AEGIS_CONCAT(Type, _U), Storage, UnorderedAllocatorParams)
#define AEGIS_TEST_RAW_TYPE_U_A(Type, Storage, Align) AEGIS_TEST_RAW_TYPE_ALIGNED(AEGIS_CONCAT(Type, _U), Storage, UnorderedAllocatorParams, Align)
// clang-format on

AEGIS_TEST_RAW_TYPE_O(Byte, uint8_t);
AEGIS_TEST_RAW_TYPE_O_A(Byte16, uint8_t, 16);
AEGIS_TEST_RAW_TYPE_U(Byte, uint8_t);
AEGIS_TEST_RAW_TYPE_U_A(Byte16, uint8_t, 16);

AEGIS_TEST_RAW_TYPE_O(Short, uint16_t);
AEGIS_TEST_RAW_TYPE_O_A(Short32, uint16_t, 32);
AEGIS_TEST_RAW_TYPE_U(Short, uint16_t);
AEGIS_TEST_RAW_TYPE_U_A(Short32, uint16_t, 32);

AEGIS_TEST_RAW_TYPE_O(Int, uint32_t);
AEGIS_TEST_RAW_TYPE_O_A(IntCache, uint32_t, 64);
AEGIS_TEST_RAW_TYPE_U(Int, uint32_t);
AEGIS_TEST_RAW_TYPE_U_A(IntCache, uint32_t, 64);

AEGIS_TEST_RAW_TYPE_O(Long, uint64_t);
AEGIS_TEST_RAW_TYPE_O_A(LongPage, uint64_t, 4096);
AEGIS_TEST_RAW_TYPE_U(Long, uint64_t);
AEGIS_TEST_RAW_TYPE_U_A(LongPage, uint64_t, 4096);

#pragma pack(push, b1, 1)
AEGIS_TEST_RAW_TYPE_O_A(IntByte, uint32_t, 1);
AEGIS_TEST_RAW_TYPE_U_A(IntByte, uint32_t, 1);
AEGIS_TEST_RAW_TYPE_O_A(LongByte, uint64_t, 1);
AEGIS_TEST_RAW_TYPE_U_A(LongByte, uint64_t, 1);
#pragma pack(pop, b1)

#pragma pack(push, b2, 1)
AEGIS_TEST_RAW_TYPE_O_A(IntShort, uint32_t, 2);
AEGIS_TEST_RAW_TYPE_U_A(IntShort, uint32_t, 2);
AEGIS_TEST_RAW_TYPE_O_A(LongShort, uint64_t, 2);
AEGIS_TEST_RAW_TYPE_U_A(LongShort, uint64_t, 2);
#pragma pack(pop, b2)

AEGIS_TEST_TYPE_O(PointerSized, sizeof(void *));
AEGIS_TEST_TYPE_U(PointerSized, sizeof(void *));

AEGIS_TEST_TYPE_O(LargerThanPointer, sizeof(void *) * 2);
AEGIS_TEST_TYPE_U(LargerThanPointer, sizeof(void *) * 2);

AEGIS_TEST_TYPE_O_A(Align16, 16, 16);
AEGIS_TEST_TYPE_U_A(Align16, 16, 16);

AEGIS_TEST_TYPE_O_A(Align64, 64, 64);
AEGIS_TEST_TYPE_U_A(Align64, 64, 64);

AEGIS_TEST_TYPE_O(Hacker, 1337);
AEGIS_TEST_TYPE_U(Hacker, 1337);

AEGIS_TEST_TYPE_O_A(Hacker_Page, 1337, 4096);
AEGIS_TEST_TYPE_U_A(Hacker_Page, 1337, 4096);

#undef AEGIS_CONCAT
#undef AEGIS_TEST_TYPE
#undef AEGIS_TEST_TYPE_ALIGNED
#undef AEGIS_TEST_RAW_TYPE
#undef AEGIS_TEST_RAW_TYPE_ALIGNED

#undef AEGIS_TEST_TYPE_O
#undef AEGIS_TEST_TYPE_O_A
#undef AEGIS_TEST_TYPE_U
#undef AEGIS_TEST_TYPE_U_A

#undef AEGIS_TEST_RAW_TYPE_O
#undef AEGIS_TEST_RAW_TYPE_O_A
#undef AEGIS_TEST_RAW_TYPE_U
#undef AEGIS_TEST_RAW_TYPE_U_A

template <typename> constexpr bool dependent_false_v = false;

template <typename T> class PoolAllocatorTest : public ::testing::Test
{
};

using PoolAllocatorTestTypes =
    ::testing::Types<PointerSized_O, PointerSized_U, LargerThanPointer_O, LargerThanPointer_U, Align16_O, Align16_U,
                     Align64_O, Align64_U, Hacker_O, Hacker_U, Hacker_Page_O, Hacker_Page_U, Byte_O, Byte_U, Byte16_O,
                     Byte16_U, Short_O, Short_U, Short32_O, Short32_U, Int_O, Int_U, IntCache_O, IntCache_U, Long_O,
                     Long_U, LongPage_O, LongPage_U, IntByte_O, IntByte_U, IntShort_O, IntShort_U, LongByte_O,
                     LongByte_U, LongShort_O, LongShort_U>;

struct PoolAllocatorTypeNames
{
    template <typename T> static std::string GetName(int)
    {
        // clang-format off
#define AEGIS_TEST_IF_NAME(Name)        if constexpr (std::is_same_v<T, Name>) return #Name;
#define AEGIS_TEST_ELIF_NAME(Name) else if constexpr (std::is_same_v<T, Name>) return #Name;
#define AEGIS_TEST_ELSE(Name)      else { static_assert(dependent_false_v<T>, "Unexpected PoolAllocator typed-test parameter"); return #Name; }
        // clang-format on

        AEGIS_TEST_IF_NAME(PointerSized_O)
        AEGIS_TEST_ELIF_NAME(PointerSized_U)
        AEGIS_TEST_ELIF_NAME(LargerThanPointer_O)
        AEGIS_TEST_ELIF_NAME(LargerThanPointer_U)
        AEGIS_TEST_ELIF_NAME(Align16_O)
        AEGIS_TEST_ELIF_NAME(Align16_U)
        AEGIS_TEST_ELIF_NAME(Align64_O)
        AEGIS_TEST_ELIF_NAME(Align64_U)
        AEGIS_TEST_ELIF_NAME(Hacker_O)
        AEGIS_TEST_ELIF_NAME(Hacker_U)
        AEGIS_TEST_ELIF_NAME(Hacker_Page_O)
        AEGIS_TEST_ELIF_NAME(Hacker_Page_U)
        AEGIS_TEST_ELIF_NAME(Byte_O)
        AEGIS_TEST_ELIF_NAME(Byte_U)
        AEGIS_TEST_ELIF_NAME(Byte16_O)
        AEGIS_TEST_ELIF_NAME(Byte16_U)
        AEGIS_TEST_ELIF_NAME(Short_O)
        AEGIS_TEST_ELIF_NAME(Short_U)
        AEGIS_TEST_ELIF_NAME(Short32_O)
        AEGIS_TEST_ELIF_NAME(Short32_U)
        AEGIS_TEST_ELIF_NAME(Int_O)
        AEGIS_TEST_ELIF_NAME(Int_U)
        AEGIS_TEST_ELIF_NAME(IntCache_O)
        AEGIS_TEST_ELIF_NAME(IntCache_U)
        AEGIS_TEST_ELIF_NAME(Long_O)
        AEGIS_TEST_ELIF_NAME(Long_U)
        AEGIS_TEST_ELIF_NAME(LongPage_O)
        AEGIS_TEST_ELIF_NAME(LongPage_U)
        AEGIS_TEST_ELIF_NAME(IntByte_O)
        AEGIS_TEST_ELIF_NAME(IntByte_U)
        AEGIS_TEST_ELIF_NAME(IntShort_O)
        AEGIS_TEST_ELIF_NAME(IntShort_U)
        AEGIS_TEST_ELIF_NAME(LongByte_O)
        AEGIS_TEST_ELIF_NAME(LongByte_U)
        AEGIS_TEST_ELIF_NAME(LongShort_O)
        AEGIS_TEST_ELIF_NAME(LongShort_U)
        AEGIS_TEST_ELSE(Unknown)

#undef AEGIS_TEST_IF_NAME
#undef AEGIS_TEST_ELIF_NAME
#undef AEGIS_TEST_ELSE
    }
};

TYPED_TEST_SUITE(PoolAllocatorTest, PoolAllocatorTestTypes, PoolAllocatorTypeNames);

enum class OracleOperation
{
    acquire,
    release_oldest,
    release_newest,
};

template <typename T>
void expect_oracle_state(std::size_t capacity, std::unordered_set<T *> const &free_slots,
                         std::unordered_set<T *> const &live_slots, std::vector<T *> const &live_order)
{
    EXPECT_EQ(capacity, free_slots.size() + live_slots.size());
    EXPECT_EQ(live_slots.size(), live_order.size());

    std::unordered_set<T *> const live_order_set(live_order.begin(), live_order.end());
    EXPECT_EQ(live_slots.size(), live_order_set.size()) << "live order contains duplicate pointers";
    EXPECT_EQ(live_slots, live_order_set);

    for (auto *slot : live_slots)
        EXPECT_FALSE(free_slots.contains(slot)) << "slot is marked both live and free";
}

template <typename T, typename TAllocator>
void run_oracle_sequence(std::uint32_t capacity, std::initializer_list<OracleOperation> operations)
{
    TAllocator allocator(capacity, 1);

    auto universe = acquire_slots(allocator, capacity);
    expect_aligned_non_null_unique(universe);
    release_slots(allocator, universe);

    std::unordered_set<T *> free_slots(universe.begin(), universe.end());
    std::unordered_set<T *> live_slots;
    std::vector<T *>        live_order;

    expect_oracle_state(capacity, free_slots, live_slots, live_order);
    expect_allocator_valid(allocator);

    for (auto operation : operations)
    {
        if (operation == OracleOperation::acquire)
        {
            ASSERT_FALSE(free_slots.empty());

            auto *slot = static_cast<T *>(allocator.acquire());
            ASSERT_NE(nullptr, slot);
            EXPECT_TRUE(is_aligned_for_type(slot));
            ASSERT_TRUE(free_slots.contains(slot)) << "allocator returned a slot that was not free in the oracle";

            free_slots.erase(slot);
            ASSERT_TRUE(live_slots.insert(slot).second);
            live_order.push_back(slot);
        }
        else
        {
            ASSERT_FALSE(live_order.empty());

            auto const slot_index =
                operation == OracleOperation::release_oldest ? std::size_t{0} : live_order.size() - 1U;
            auto *slot = live_order[slot_index];
            live_order.erase(live_order.begin() + static_cast<std::ptrdiff_t>(slot_index));

            allocator.release(slot);
            ASSERT_EQ(1U, live_slots.erase(slot));
            ASSERT_TRUE(free_slots.insert(slot).second);
        }

        expect_oracle_state(capacity, free_slots, live_slots, live_order);
        expect_allocator_valid(allocator);
    }
}

template <typename T, typename TAllocator> void run_random_oracle_sequence()
{
    constexpr std::uint32_t capacity = 8;
    constexpr std::uint32_t steps    = 256;

    TAllocator allocator(capacity, 1);

    auto universe = acquire_slots(allocator, capacity);
    expect_aligned_non_null_unique(universe);
    release_slots(allocator, universe);

    std::unordered_set<T *> free_slots(universe.begin(), universe.end());
    std::unordered_set<T *> live_slots;
    std::vector<T *>        live_order;

    std::random_device                 rd;
    std::mt19937                       random_engine(rd());
    std::uniform_int_distribution<int> dist{0, 1};

    expect_allocator_valid(allocator);

    for (std::uint32_t step = 0; step < steps; ++step)
    {
        bool const should_acquire = live_slots.empty() || (!free_slots.empty() && dist(random_engine) == 0);

        if (should_acquire)
        {
            auto *slot = static_cast<T *>(allocator.acquire());
            ASSERT_NE(nullptr, slot) << "step " << step;
            EXPECT_TRUE(is_aligned_for_type(slot)) << "step " << step;
            ASSERT_TRUE(free_slots.contains(slot)) << "step " << step;

            free_slots.erase(slot);
            ASSERT_TRUE(live_slots.insert(slot).second) << "step " << step;
            live_order.push_back(slot);
        }
        else
        {
            auto const slot_index =
                std::uniform_int_distribution<std::size_t>{0, live_order.size() - 1U}(random_engine);
            auto *slot = live_order[slot_index];
            live_order.erase(live_order.begin() + static_cast<std::ptrdiff_t>(slot_index));

            allocator.release(slot);
            ASSERT_EQ(1U, live_slots.erase(slot)) << "step " << step;
            ASSERT_TRUE(free_slots.insert(slot).second) << "step " << step;
        }

        expect_oracle_state(capacity, free_slots, live_slots, live_order);
        expect_allocator_valid(allocator);
    }
}

TYPED_TEST(PoolAllocatorTest, AcquireReturnsAlignedNonNullUniqueLiveSlots)
{
    constexpr std::uint32_t         slots_per_arena = 8;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);

    auto const slots = acquire_slots(allocator, 5);

    expect_aligned_non_null_unique(slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, CanAcquireEntirePreallocatedArena)
{
    constexpr std::uint32_t         slots_per_arena = 8;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);

    auto const slots = acquire_slots(allocator, slots_per_arena);

    expect_aligned_non_null_unique(slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, AcquireGrowsAfterEntireArenaIsExhausted)
{
    constexpr std::uint32_t         slots_per_arena = 4;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);

    auto slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(slots);

    auto *extra_slot = static_cast<TypeParam *>(allocator.acquire());
    ASSERT_NE(nullptr, extra_slot);
    EXPECT_TRUE(is_aligned_for_type(extra_slot));

    std::unordered_set<TypeParam *> const live_slots(slots.begin(), slots.end());
    EXPECT_FALSE(live_slots.contains(extra_slot));
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, ReleaseEntireArenaThenReacquireReusesSamePointers)
{
    constexpr std::uint32_t         slots_per_arena = 8;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);

    auto const original_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(original_slots);

    release_slots(allocator, original_slots);
    expect_allocator_valid(allocator);

    auto const reacquired_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(original_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, AcquireReleaseReusesArenaWhenNoArenasWerePreallocated)
{
    constexpr std::uint32_t         slots_per_arena = 8;
    typename TypeParam::allocator_t allocator(slots_per_arena, 0);

    auto const original_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(original_slots);

    release_slots(allocator, original_slots);
    expect_allocator_valid(allocator);

    auto const reacquired_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(original_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, CanAcquireAndReuseAcrossMultiplePreallocatedArenas)
{
    constexpr std::uint32_t         slots_per_arena          = 3;
    constexpr std::uint32_t         preallocated_arena_count = 3;
    constexpr std::uint32_t         total_slots              = slots_per_arena * preallocated_arena_count;
    typename TypeParam::allocator_t allocator(slots_per_arena, preallocated_arena_count);

    auto const original_slots = acquire_slots(allocator, total_slots);
    expect_aligned_non_null_unique(original_slots);

    release_slots(allocator, original_slots);
    expect_allocator_valid(allocator);

    auto const reacquired_slots = acquire_slots(allocator, total_slots);
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(original_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, RepeatedFullArenaAcquireReleaseCyclesRemainStable)
{
    constexpr std::uint32_t         slots_per_arena = 8;
    constexpr std::uint32_t         cycles          = 5;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);

    std::vector<TypeParam *> baseline_slots;

    for (std::uint32_t cycle = 0; cycle < cycles; ++cycle)
    {
        auto slots = acquire_slots(allocator, slots_per_arena);
        expect_aligned_non_null_unique(slots);

        if (baseline_slots.empty())
            baseline_slots = slots;
        else
            expect_same_pointer_set(baseline_slots, slots);

        if (cycle % 2 == 0)
            release_slots(allocator, slots);
        else
            release_slots_reversed(allocator, slots);

        expect_allocator_valid(allocator);
    }
}

TYPED_TEST(PoolAllocatorTest, ReleaseFirstMiddleAndLastSlotsThenReacquireThem)
{
    constexpr std::uint32_t         slots_per_arena = 7;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);

    auto const slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(slots);

    std::vector<TypeParam *> const released_slots{slots.front(), slots[slots.size() / 2], slots.back()};
    release_slots(allocator, released_slots);
    expect_allocator_valid(allocator);

    auto const reacquired_slots = acquire_slots(allocator, released_slots.size());
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(released_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, MultipleReleasedSlotsAreReusedWithoutLiveDuplication)
{
    constexpr std::uint32_t         slots_per_arena = 8;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);

    auto const slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(slots);

    std::vector<TypeParam *> const released_slots{slots[6], slots[1], slots[4]};
    release_slots(allocator, released_slots);
    expect_allocator_valid(allocator);

    auto const reacquired_slots = acquire_slots(allocator, released_slots.size());
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(released_slots, reacquired_slots);

    std::vector<TypeParam *> live_slots;
    live_slots.reserve(slots.size());
    for (auto *slot : slots)
    {
        if (std::find(released_slots.begin(), released_slots.end(), slot) == released_slots.end())
            live_slots.push_back(slot);
    }
    live_slots.insert(live_slots.end(), reacquired_slots.begin(), reacquired_slots.end());

    expect_aligned_non_null_unique(live_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, NullAndForeignReleaseDoNotCorruptAllocator)
{
    constexpr std::uint32_t         slots_per_arena = 8;
    typename TypeParam::allocator_t allocator(slots_per_arena, 1);
    TypeParam                       foreign_slot{};

    allocator.release(nullptr);
    allocator.release(&foreign_slot);
    expect_allocator_valid(allocator);

    auto const slots = acquire_slots(allocator, slots_per_arena);

    expect_aligned_non_null_unique(slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, SmallStateOracleSequencesPreserveInvariants)
{
    using Op = OracleOperation;

    run_oracle_sequence<TypeParam, typename TypeParam::allocator_t>(
        1, {Op::acquire, Op::release_newest, Op::acquire, Op::release_oldest});
    run_oracle_sequence<TypeParam, typename TypeParam::allocator_t>(2, {Op::acquire, Op::acquire, Op::release_oldest,
                                                                        Op::acquire, Op::release_newest,
                                                                        Op::release_newest, Op::acquire, Op::acquire});
    run_oracle_sequence<TypeParam, typename TypeParam::allocator_t>(
        3, {Op::acquire, Op::acquire, Op::acquire, Op::release_oldest, Op::acquire, Op::release_newest,
            Op::release_oldest, Op::acquire});
    run_oracle_sequence<TypeParam, typename TypeParam::allocator_t>(
        4, {Op::acquire, Op::acquire, Op::acquire, Op::acquire, Op::release_oldest, Op::release_newest, Op::acquire,
            Op::release_oldest, Op::acquire});
}

TYPED_TEST(PoolAllocatorTest, FixedSeedRandomOracleSequencePreservesInvariants)
{ run_random_oracle_sequence<TypeParam, typename TypeParam::allocator_t>(); }
} // namespace
