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

template <typename T> std::vector<T *> acquire_slots(PoolAllocator<T> &allocator, std::size_t count)
{
    std::vector<T *> slots(count);

    for (std::size_t i = 0; i < count; ++i)
        slots[i] = allocator.acquire();

    return slots;
}

template <typename T> void release_slots(PoolAllocator<T> &allocator, std::vector<T *> const &slots)
{
    for (auto *slot : slots)
    {
        if (slot)
            allocator.release(slot);
    }
}

template <typename T> void release_slots_reversed(PoolAllocator<T> &allocator, std::vector<T *> const &slots)
{
    for (auto slot = slots.rbegin(); slot != slots.rend(); ++slot)
    {
        if (*slot)
            allocator.release(*slot);
    }
}

template <typename T>::testing::AssertionResult is_aligned_for_type(T *slot)
{
    auto const address = reinterpret_cast<std::uintptr_t>(slot);
    if (address % alignof(T) == 0)
        return ::testing::AssertionSuccess();

    return ::testing::AssertionFailure() << "address " << static_cast<void const *>(slot)
                                         << " is not aligned to alignof(T)=" << alignof(T);
}

template <typename T> void expect_aligned_non_null_unique(std::vector<T *> const &slots)
{
    std::unordered_set<T *> unique_slots;
    unique_slots.reserve(slots.size());

    for (std::size_t i = 0; i < slots.size(); ++i)
    {
        auto *slot = slots[i];
        if (!slot)
        {
            ADD_FAILURE() << "slot " << i << " is null";
            continue;
        }

        EXPECT_TRUE(is_aligned_for_type(slot)) << "slot " << i;
        EXPECT_TRUE(unique_slots.insert(slot).second) << "slot " << i << " duplicates a live allocation";
    }
}

template <typename T>
void expect_same_pointer_set(std::vector<T *> const &expected_slots, std::vector<T *> const &actual_slots)
{
    EXPECT_EQ(expected_slots.size(), actual_slots.size());

    std::unordered_set<T *> const expected_set(expected_slots.begin(), expected_slots.end());
    std::unordered_set<T *> const actual_set(actual_slots.begin(), actual_slots.end());

    EXPECT_EQ(expected_slots.size(), expected_set.size()) << "expected slot list contains duplicates";
    EXPECT_EQ(actual_slots.size(), actual_set.size()) << "actual slot list contains duplicates";
    EXPECT_EQ(expected_set, actual_set);
}

template <typename T> void expect_allocator_valid(PoolAllocator<T> const &allocator)
{ EXPECT_TRUE(allocator.debug_validate()); }
} // namespace