#include <aegis/allocators/pool_allocator.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <random>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace
{
using aegis::allocators::PoolAllocator;

struct PointerSized
{
    std::byte bytes[sizeof(void *)];
};

struct LargerThanPointer
{
    std::byte bytes[sizeof(void *) * 2];
};

struct alignas(16) Align16
{
    std::byte bytes[16];
};

struct alignas(64) Align64
{
    std::byte bytes[64];
};

template <typename> constexpr bool dependent_false_v = false;

template <typename T> class PoolAllocatorTest : public ::testing::Test
{
};

using PoolAllocatorTestTypes =
    ::testing::Types<std::byte, std::uint16_t, std::uint32_t, std::uint64_t, PointerSized, LargerThanPointer,
                     Align16, Align64>;

struct PoolAllocatorTypeNames
{
    template <typename T> static std::string GetName(int)
    {
        if constexpr (std::is_same_v<T, std::byte>)
            return "Byte";
        else if constexpr (std::is_same_v<T, std::uint16_t>)
            return "Uint16";
        else if constexpr (std::is_same_v<T, std::uint32_t>)
            return "Uint32";
        else if constexpr (std::is_same_v<T, std::uint64_t>)
            return "Uint64";
        else if constexpr (std::is_same_v<T, PointerSized>)
            return "PointerSized";
        else if constexpr (std::is_same_v<T, LargerThanPointer>)
            return "LargerThanPointer";
        else if constexpr (std::is_same_v<T, Align16>)
            return "Align16";
        else if constexpr (std::is_same_v<T, Align64>)
            return "Align64";
        else
        {
            static_assert(dependent_false_v<T>, "Unexpected PoolAllocator typed-test parameter");
            return {};
        }
    }
};

TYPED_TEST_SUITE(PoolAllocatorTest, PoolAllocatorTestTypes, PoolAllocatorTypeNames);

template <typename T> std::vector<T *> acquire_slots(PoolAllocator<T> &allocator, std::size_t count)
{
    std::vector<T *> slots;
    slots.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
        slots.push_back(static_cast<T *>(allocator.acquire()));

    return slots;
}

template <typename T> void release_slots(PoolAllocator<T> &allocator, const std::vector<T *> &slots)
{
    for (auto *slot : slots)
    {
        if (slot != nullptr)
            allocator.release(slot);
    }
}

template <typename T> void release_slots_reversed(PoolAllocator<T> &allocator, const std::vector<T *> &slots)
{
    for (auto slot = slots.rbegin(); slot != slots.rend(); ++slot)
    {
        if (*slot != nullptr)
            allocator.release(*slot);
    }
}

template <typename T> ::testing::AssertionResult is_aligned_for_type(T *slot)
{
    const auto address = reinterpret_cast<std::uintptr_t>(slot);
    if (address % alignof(T) == 0)
        return ::testing::AssertionSuccess();

    return ::testing::AssertionFailure() << "address " << static_cast<const void *>(slot)
                                         << " is not aligned to alignof(T)=" << alignof(T);
}

template <typename T> void expect_aligned_non_null_unique(const std::vector<T *> &slots)
{
    std::unordered_set<T *> unique_slots;
    unique_slots.reserve(slots.size());

    for (std::size_t i = 0; i < slots.size(); ++i)
    {
        auto *slot = slots[i];
        if (slot == nullptr)
        {
            ADD_FAILURE() << "slot " << i << " is null";
            continue;
        }

        EXPECT_TRUE(is_aligned_for_type(slot)) << "slot " << i;
        EXPECT_TRUE(unique_slots.insert(slot).second) << "slot " << i << " duplicates a live allocation";
    }
}

template <typename T>
void expect_same_pointer_set(const std::vector<T *> &expected_slots, const std::vector<T *> &actual_slots)
{
    EXPECT_EQ(expected_slots.size(), actual_slots.size());

    const std::unordered_set<T *> expected_set(expected_slots.begin(), expected_slots.end());
    const std::unordered_set<T *> actual_set(actual_slots.begin(), actual_slots.end());

    EXPECT_EQ(expected_slots.size(), expected_set.size()) << "expected slot list contains duplicates";
    EXPECT_EQ(actual_slots.size(), actual_set.size()) << "actual slot list contains duplicates";
    EXPECT_EQ(expected_set, actual_set);
}

template <typename T> void expect_allocator_valid(const PoolAllocator<T> &allocator)
{
    EXPECT_TRUE(allocator.debug_validate());
}

enum class OracleOperation
{
    acquire,
    release_oldest,
    release_newest,
};

template <typename T>
void expect_oracle_state(std::size_t capacity, const std::unordered_set<T *> &free_slots,
                         const std::unordered_set<T *> &live_slots, const std::vector<T *> &live_order)
{
    EXPECT_EQ(capacity, free_slots.size() + live_slots.size());
    EXPECT_EQ(live_slots.size(), live_order.size());

    const std::unordered_set<T *> live_order_set(live_order.begin(), live_order.end());
    EXPECT_EQ(live_slots.size(), live_order_set.size()) << "live order contains duplicate pointers";
    EXPECT_EQ(live_slots, live_order_set);

    for (auto *slot : live_slots)
        EXPECT_FALSE(free_slots.contains(slot)) << "slot is marked both live and free";
}

template <typename T>
void run_oracle_sequence(std::uint32_t capacity, std::initializer_list<OracleOperation> operations)
{
    PoolAllocator<T> allocator(capacity, 1);

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

            const auto slot_index =
                operation == OracleOperation::release_oldest ? std::size_t{0} : live_order.size() - 1U;
            auto      *slot       = live_order[slot_index];
            live_order.erase(live_order.begin() + static_cast<std::ptrdiff_t>(slot_index));

            allocator.release(slot);
            ASSERT_EQ(1U, live_slots.erase(slot));
            ASSERT_TRUE(free_slots.insert(slot).second);
        }

        expect_oracle_state(capacity, free_slots, live_slots, live_order);
        expect_allocator_valid(allocator);
    }
}

template <typename T> void run_random_oracle_sequence()
{
    constexpr std::uint32_t capacity = 8;
    constexpr std::uint32_t steps    = 256;

    PoolAllocator<T> allocator(capacity, 1);

    auto universe = acquire_slots(allocator, capacity);
    expect_aligned_non_null_unique(universe);
    release_slots(allocator, universe);

    std::unordered_set<T *> free_slots(universe.begin(), universe.end());
    std::unordered_set<T *> live_slots;
    std::vector<T *>        live_order;
    std::mt19937            random_engine{0xAE615U};

    expect_allocator_valid(allocator);

    for (std::uint32_t step = 0; step < steps; ++step)
    {
        const bool should_acquire =
            live_slots.empty() || (!free_slots.empty() && std::uniform_int_distribution<int>{0, 1}(random_engine) == 0);

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
            const auto slot_index =
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
    constexpr std::uint32_t slots_per_arena = 8;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

    const auto slots = acquire_slots(allocator, 5);

    expect_aligned_non_null_unique(slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, CanAcquireEntirePreallocatedArena)
{
    constexpr std::uint32_t slots_per_arena = 8;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

    const auto slots = acquire_slots(allocator, slots_per_arena);

    expect_aligned_non_null_unique(slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, AcquireGrowsAfterEntireArenaIsExhausted)
{
    constexpr std::uint32_t slots_per_arena = 4;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

    auto slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(slots);

    auto *extra_slot = static_cast<TypeParam *>(allocator.acquire());
    ASSERT_NE(nullptr, extra_slot);
    EXPECT_TRUE(is_aligned_for_type(extra_slot));

    const std::unordered_set<TypeParam *> live_slots(slots.begin(), slots.end());
    EXPECT_FALSE(live_slots.contains(extra_slot));
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, ReleaseEntireArenaThenReacquireReusesSamePointers)
{
    constexpr std::uint32_t slots_per_arena = 8;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

    const auto original_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(original_slots);

    release_slots(allocator, original_slots);
    expect_allocator_valid(allocator);

    const auto reacquired_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(original_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, AcquireReleaseReusesArenaWhenNoArenasWerePreallocated)
{
    constexpr std::uint32_t slots_per_arena = 8;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 0);

    const auto original_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(original_slots);

    release_slots(allocator, original_slots);
    expect_allocator_valid(allocator);

    const auto reacquired_slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(original_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, CanAcquireAndReuseAcrossMultiplePreallocatedArenas)
{
    constexpr std::uint32_t slots_per_arena          = 3;
    constexpr std::uint32_t preallocated_arena_count = 3;
    constexpr std::uint32_t total_slots              = slots_per_arena * preallocated_arena_count;
    PoolAllocator<TypeParam> allocator(slots_per_arena, preallocated_arena_count);

    const auto original_slots = acquire_slots(allocator, total_slots);
    expect_aligned_non_null_unique(original_slots);

    release_slots(allocator, original_slots);
    expect_allocator_valid(allocator);

    const auto reacquired_slots = acquire_slots(allocator, total_slots);
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(original_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, RepeatedFullArenaAcquireReleaseCyclesRemainStable)
{
    constexpr std::uint32_t slots_per_arena = 8;
    constexpr std::uint32_t cycles          = 5;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

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
    constexpr std::uint32_t slots_per_arena = 7;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

    const auto slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(slots);

    const std::vector<TypeParam *> released_slots{slots.front(), slots[slots.size() / 2], slots.back()};
    release_slots(allocator, released_slots);
    expect_allocator_valid(allocator);

    const auto reacquired_slots = acquire_slots(allocator, released_slots.size());
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(released_slots, reacquired_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, MultipleReleasedSlotsAreReusedWithoutLiveDuplication)
{
    constexpr std::uint32_t slots_per_arena = 8;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

    const auto slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(slots);

    const std::vector<TypeParam *> released_slots{slots[6], slots[1], slots[4]};
    release_slots(allocator, released_slots);
    expect_allocator_valid(allocator);

    const auto reacquired_slots = acquire_slots(allocator, released_slots.size());
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

TYPED_TEST(PoolAllocatorTest, DoubleReleaseDoesNotCorruptAllocator)
{
    constexpr std::uint32_t slots_per_arena = 8;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);

    const auto slots = acquire_slots(allocator, slots_per_arena);
    expect_aligned_non_null_unique(slots);

    allocator.release(slots[1]);
    allocator.release(slots[1]);
    allocator.release(slots[5]);
    expect_allocator_valid(allocator);

    const std::vector<TypeParam *> expected_reused_slots{slots[1], slots[5]};
    const auto                     reacquired_slots = acquire_slots(allocator, expected_reused_slots.size());
    expect_aligned_non_null_unique(reacquired_slots);
    expect_same_pointer_set(expected_reused_slots, reacquired_slots);

    std::vector<TypeParam *> live_slots;
    live_slots.reserve(slots.size());
    for (auto *slot : slots)
    {
        if (std::find(expected_reused_slots.begin(), expected_reused_slots.end(), slot) == expected_reused_slots.end())
            live_slots.push_back(slot);
    }
    live_slots.insert(live_slots.end(), reacquired_slots.begin(), reacquired_slots.end());

    expect_aligned_non_null_unique(live_slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, NullAndForeignReleaseDoNotCorruptAllocator)
{
    constexpr std::uint32_t slots_per_arena = 8;
    PoolAllocator<TypeParam> allocator(slots_per_arena, 1);
    TypeParam                foreign_slot{};

    allocator.release(nullptr);
    allocator.release(&foreign_slot);
    expect_allocator_valid(allocator);

    const auto slots = acquire_slots(allocator, slots_per_arena);

    expect_aligned_non_null_unique(slots);
    expect_allocator_valid(allocator);
}

TYPED_TEST(PoolAllocatorTest, SmallStateOracleSequencesPreserveInvariants)
{
    using Op = OracleOperation;

    run_oracle_sequence<TypeParam>(1, {Op::acquire, Op::release_newest, Op::acquire, Op::release_oldest});
    run_oracle_sequence<TypeParam>(2, {Op::acquire, Op::acquire, Op::release_oldest, Op::acquire,
                                       Op::release_newest, Op::release_newest, Op::acquire, Op::acquire});
    run_oracle_sequence<TypeParam>(3, {Op::acquire, Op::acquire, Op::acquire, Op::release_oldest, Op::acquire,
                                       Op::release_newest, Op::release_oldest, Op::acquire});
    run_oracle_sequence<TypeParam>(4, {Op::acquire, Op::acquire, Op::acquire, Op::acquire, Op::release_oldest,
                                       Op::release_newest, Op::acquire, Op::release_oldest, Op::acquire});
}

TYPED_TEST(PoolAllocatorTest, FixedSeedRandomOracleSequencePreservesInvariants)
{
    run_random_oracle_sequence<TypeParam>();
}
} // namespace
