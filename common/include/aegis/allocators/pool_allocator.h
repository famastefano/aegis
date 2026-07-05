#pragma once

#include <cmath>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

#ifdef AEGIS_BUILD_TEST
    #include <unordered_set>
#endif

#if defined(__has_include)
    #if __has_include(<sanitizer/asan_interface.h>)
        #include <sanitizer/asan_interface.h>
    #endif
#endif

#ifndef ASAN_POISON_MEMORY_REGION
    #define ASAN_POISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#endif

#ifndef ASAN_UNPOISON_MEMORY_REGION
    #define ASAN_UNPOISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#endif

namespace aegis::allocators
{
/*
 * Pool allocator for type T.
 * Memory layout is a collection of equally sized arenas.
 * An in-place free list is used, so the smallest block size will be sizeof(void*).
 */
template <typename T> class PoolAllocator
{
    using free_list_value_t = unsigned char *;
    using slot_type_t       = typename std::conditional_t<sizeof(free_list_value_t) >= sizeof(T), free_list_value_t, T>;
    inline static constexpr auto slot_alignment = alignof(T) > alignof(free_list_value_t)
                                                      ? std::align_val_t{alignof(T)}
                                                      : std::align_val_t{alignof(free_list_value_t)};

    // The arena has proper alignment, but slots in the middle might not.
    // We calculate the actual slot size to maintain the returned pointer aligned.
    static constexpr auto calculate_slot_size()
    {
        if constexpr (sizeof(slot_type_t) % static_cast<std::size_t>(slot_alignment) == 0)
            return sizeof(slot_type_t);
        else if constexpr (sizeof(slot_type_t) < static_cast<std::size_t>(slot_alignment))
            return static_cast<std::size_t>(slot_alignment);
        else
            return sizeof(slot_type_t) + (sizeof(slot_type_t) - alignof(T));
    }

    inline static constexpr auto slot_size = calculate_slot_size();

    struct arena_deleter_t
    {
        void operator()(void *p)
        { ::operator delete[](p, slot_alignment); }
    };
    using arena_t = typename std::unique_ptr<unsigned char[], arena_deleter_t>;

    constexpr free_list_value_t slot_addr(arena_t const &arena, std::size_t pos) const
    { return arena.get() + (slot_size * pos); }

    constexpr free_list_value_t prev_slot_addr(free_list_value_t slot) const
    { return slot - slot_size; }

    constexpr free_list_value_t next_slot_addr(free_list_value_t slot) const
    { return slot + slot_size; }

    constexpr free_list_value_t *as_ptr(free_list_value_t slot) const
    { return std::launder(reinterpret_cast<free_list_value_t *>(slot)); }

  public:
    static_assert(!std::is_pointer_v<T> || sizeof(T) <= sizeof(void *),
                  "PoolAllocator<T*> doesn't support polymorphic objects.");

    PoolAllocator(std::uint32_t slots_per_arena, std::uint32_t prellocated_arenas_count = 0)
        : arenas_(prellocated_arenas_count), slots_per_arena_(slots_per_arena)
    {
        for (auto &arena : arenas_)
            arena = make_arena();

        init_free_list();
    }

    T *acquire()
    {
        if (free_list_head_)
        {
            ASAN_UNPOISON_MEMORY_REGION(free_list_head_, slot_size);
            auto slot       = free_list_head_;
            free_list_head_ = *as_ptr(free_list_head_);
            return std::launder(reinterpret_cast<T *>(slot));
        }

        auto &arena = arenas_.emplace_back(make_arena());
        init_free_list(arena);
        free_list_head_ = arena.get();
        return acquire();
    }

    void release(T *p)
    {
        // Not allocated by us
        if (!p || !find_arena(p))
            return;

        if (!free_list_head_)
        {
            free_list_head_          = reinterpret_cast<free_list_value_t>(p);
            *as_ptr(free_list_head_) = nullptr;
            ASAN_POISON_MEMORY_REGION(free_list_head_, slot_size);
            return;
        }

        free_list_value_t p_slot    = reinterpret_cast<free_list_value_t>(p);
        free_list_value_t prev_slot = nullptr;
        free_list_value_t curr_slot = free_list_head_;
        while (curr_slot)
        {
            ASAN_UNPOISON_MEMORY_REGION(curr_slot, slot_size);
            free_list_value_t next_slot = *as_ptr(curr_slot);
            // Double-free prevention
            if (p_slot == curr_slot || p_slot == next_slot)
            {
                ASAN_POISON_MEMORY_REGION(curr_slot, slot_size);
                return;
            }

            if (p_slot < curr_slot)
            {
                if (prev_slot)
                {
                    ASAN_UNPOISON_MEMORY_REGION(prev_slot, slot_size);
                    *as_ptr(prev_slot) = p_slot;
                    ASAN_POISON_MEMORY_REGION(prev_slot, slot_size);
                }
                else
                {
                    free_list_head_ = p_slot;
                }
                ASAN_UNPOISON_MEMORY_REGION(p_slot, slot_size);
                *as_ptr(p_slot) = curr_slot;
                ASAN_POISON_MEMORY_REGION(p_slot, slot_size);
                return;
            }
            else if (curr_slot < p_slot && p_slot < next_slot)
            {
                ASAN_UNPOISON_MEMORY_REGION(curr_slot, slot_size);
                ASAN_UNPOISON_MEMORY_REGION(p_slot, slot_size);
                *as_ptr(curr_slot) = p_slot;
                *as_ptr(p_slot)    = next_slot;
                ASAN_POISON_MEMORY_REGION(curr_slot, slot_size);
                ASAN_POISON_MEMORY_REGION(p_slot, slot_size);
                return;
            }
            else if (!next_slot)
            {
                ASAN_UNPOISON_MEMORY_REGION(curr_slot, slot_size);
                ASAN_UNPOISON_MEMORY_REGION(p_slot, slot_size);
                *as_ptr(curr_slot) = p_slot;
                *as_ptr(p_slot)    = nullptr;
                ASAN_POISON_MEMORY_REGION(curr_slot, slot_size);
                ASAN_POISON_MEMORY_REGION(p_slot, slot_size);
                return;
            }
            ASAN_POISON_MEMORY_REGION(curr_slot, slot_size);

            prev_slot = curr_slot;
            curr_slot = next_slot;
        }
    }

#ifdef AEGIS_BUILD_TEST
    bool debug_validate() const
    {
        std::unordered_set<free_list_value_t> traversed_slots;

        free_list_value_t slot = free_list_head_;
        while (slot)
        {
            auto const slot_address = reinterpret_cast<std::uintptr_t>(slot);
            auto const alignment    = static_cast<std::size_t>(slot_alignment);
            if (slot_address & (alignment - 1U))
                return false;

            if (!find_arena(slot))
                return false;

            if (traversed_slots.contains(slot))
                return false;

            traversed_slots.emplace_hint(traversed_slots.end(), slot);

            ASAN_UNPOISON_MEMORY_REGION(slot, slot_size);
            slot = *as_ptr(slot);
            ASAN_POISON_MEMORY_REGION(slot, slot_size);
        }
        return true;
    }

    bool debug_is_empty() const
    { return debug_calculate_allocated_ratio() == 0.0f; }

    bool debug_is_full() const
    { return !free_list_head_; }

    float debug_calculate_allocated_ratio() const
    {
        std::uint64_t const allocated_slots = arenas_.size() * slots_per_arena_;
        std::uint64_t       free_slots      = 0;

        free_list_value_t slot = free_list_head_;
        while (slot)
        {
            ASAN_UNPOISON_MEMORY_REGION(slot, slot_size);
            slot = *as_ptr(slot);
            ASAN_POISON_MEMORY_REGION(slot, slot_size);
            ++free_slots;
        }
        return float(free_slots) / allocated_slots;
    }
#endif

  private:
    arena_t make_arena()
    {
        auto const arena_size = slot_size * slots_per_arena_;
        void      *mem        = ::operator new[](arena_size, slot_alignment);
        ASAN_POISON_MEMORY_REGION(mem, arena_size);
        return arena_t(static_cast<unsigned char *>(mem));
    }

    arena_t const *find_arena(void *p) const
    {
        auto const p_addr = reinterpret_cast<std::uintptr_t>(p);
        for (auto const &arena : arenas_)
        {
            auto const begin = reinterpret_cast<std::uintptr_t>(slot_addr(arena, 0));
            auto const end   = reinterpret_cast<std::uintptr_t>(slot_addr(arena, slots_per_arena_));
            if (begin <= p_addr && p_addr < end)
                return &arena;
        }
        return nullptr;
    }

    void init_free_list(arena_t &arena)
    {
        free_list_value_t       begin = slot_addr(arena, 0);
        free_list_value_t const end   = slot_addr(arena, slots_per_arena_);
        while (begin != end)
        {
            ASAN_UNPOISON_MEMORY_REGION(begin, slot_size);
            *as_ptr(begin) = next_slot_addr(begin);
            ASAN_POISON_MEMORY_REGION(begin, slot_size);
            begin = next_slot_addr(begin);
        }
        free_list_value_t *tail = as_ptr(prev_slot_addr(end));
        ASAN_UNPOISON_MEMORY_REGION(tail, slot_size);
        *tail = nullptr;
        ASAN_POISON_MEMORY_REGION(tail, slot_size);
    }

    void init_free_list()
    {
        for (std::size_t i = 0; i < arenas_.size(); ++i)
        {
            auto &curr_arena = arenas_[i];
            init_free_list(curr_arena);

            free_list_value_t *tail = as_ptr(slot_addr(curr_arena, slots_per_arena_ - 1));
            ASAN_UNPOISON_MEMORY_REGION(tail, slot_size);
            if (i + 1 < arenas_.size())
                *tail = slot_addr(arenas_[i + 1], 0);
            else
                *tail = nullptr;
            ASAN_POISON_MEMORY_REGION(tail, slot_size);
        }

        if (!arenas_.empty())
            free_list_head_ = slot_addr(arenas_.front(), 0);
    }

    std::vector<arena_t> arenas_;
    free_list_value_t    free_list_head_{};
    std::uint32_t        slots_per_arena_{};

#undef ASAN_UNPOISON_MEMORY_REGION
#undef ASAN_POISON_MEMORY_REGION
};
} // namespace aegis::allocators
