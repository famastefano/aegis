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
    using free_list_value_t = void *;
    using slot_type_t       = typename std::conditional_t<sizeof(free_list_value_t) >= sizeof(T), free_list_value_t, T>;
    inline static constexpr std::align_val_t slot_alignment = alignof(T) > alignof(free_list_value_t)
                                                                  ? std::align_val_t{alignof(T)}
                                                                  : std::align_val_t{alignof(free_list_value_t)};

    struct arena_deleter_t
    {
        void operator()(slot_type_t *p)
        { ::operator delete(p, slot_alignment); }
    };
    using arena_t = typename std::unique_ptr<slot_type_t, arena_deleter_t>;

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
            ASAN_UNPOISON_MEMORY_REGION(free_list_head_, sizeof(slot_type_t));
            auto slot       = free_list_head_;
            free_list_head_ = *reinterpret_cast<free_list_value_t *>(free_list_head_);
            return static_cast<T *>(slot);
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
            free_list_head_                                         = p;
            *reinterpret_cast<free_list_value_t *>(free_list_head_) = nullptr;
            ASAN_POISON_MEMORY_REGION(free_list_head_, sizeof(slot_type_t));
            return;
        }

        free_list_value_t p_slot    = p;
        free_list_value_t prev_slot = nullptr;
        free_list_value_t curr_slot = free_list_head_;
        while (curr_slot)
        {
            ASAN_UNPOISON_MEMORY_REGION(curr_slot, sizeof(slot_type_t));
            free_list_value_t next_slot = *reinterpret_cast<free_list_value_t *>(curr_slot);
            // Double-free prevention
            if (p_slot == curr_slot || p_slot == next_slot)
            {
                ASAN_POISON_MEMORY_REGION(curr_slot, sizeof(slot_type_t));
                return;
            }

            if (p_slot < curr_slot)
            {
                if (prev_slot)
                {
                    ASAN_UNPOISON_MEMORY_REGION(prev_slot, sizeof(slot_type_t));
                    *reinterpret_cast<free_list_value_t *>(prev_slot) = p_slot;
                    ASAN_POISON_MEMORY_REGION(prev_slot, sizeof(slot_type_t));
                }
                else
                {
                    free_list_head_ = p_slot;
                }
                ASAN_UNPOISON_MEMORY_REGION(p_slot, sizeof(slot_type_t));
                *reinterpret_cast<free_list_value_t *>(p_slot) = curr_slot;
                ASAN_POISON_MEMORY_REGION(p_slot, sizeof(slot_type_t));
                return;
            }
            else if (curr_slot < p_slot && p_slot < next_slot)
            {
                ASAN_UNPOISON_MEMORY_REGION(curr_slot, sizeof(slot_type_t));
                ASAN_UNPOISON_MEMORY_REGION(p_slot, sizeof(slot_type_t));
                *reinterpret_cast<free_list_value_t *>(curr_slot) = p_slot;
                *reinterpret_cast<free_list_value_t *>(p_slot)    = next_slot;
                ASAN_POISON_MEMORY_REGION(curr_slot, sizeof(slot_type_t));
                ASAN_POISON_MEMORY_REGION(p_slot, sizeof(slot_type_t));
                return;
            }
            else if (!next_slot)
            {
                ASAN_UNPOISON_MEMORY_REGION(curr_slot, sizeof(slot_type_t));
                ASAN_UNPOISON_MEMORY_REGION(p_slot, sizeof(slot_type_t));
                *reinterpret_cast<free_list_value_t *>(curr_slot) = p_slot;
                *reinterpret_cast<free_list_value_t *>(p_slot)    = nullptr;
                ASAN_POISON_MEMORY_REGION(curr_slot, sizeof(slot_type_t));
                ASAN_POISON_MEMORY_REGION(p_slot, sizeof(slot_type_t));
                return;
            }
            ASAN_POISON_MEMORY_REGION(curr_slot, sizeof(slot_type_t));

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
            auto const alignment    = static_cast<std::uintptr_t>(slot_alignment);
            if (slot_address & (alignment - 1U))
                return false;

            if (!find_arena(slot))
                return false;

            if (traversed_slots.contains(slot))
                return false;

            traversed_slots.emplace_hint(traversed_slots.end(), slot);

            ASAN_UNPOISON_MEMORY_REGION(slot, sizeof(slot_type_t));
            slot = *reinterpret_cast<free_list_value_t *>(slot);
            ASAN_POISON_MEMORY_REGION(slot, sizeof(slot_type_t));
        }
        return true;
    }
#endif

  private:
    arena_t make_arena()
    {
        void *mem = ::operator new(sizeof(slot_type_t) * slots_per_arena_, slot_alignment);
        ASAN_POISON_MEMORY_REGION(mem, sizeof(slot_type_t) * slots_per_arena_);
        return arena_t(static_cast<slot_type_t *>(mem));
    }

    arena_t const *find_arena(void *p) const
    {
        for (auto const &arena : arenas_)
        {
            void *const begin = arena.get();
            void *const end   = arena.get() + slots_per_arena_;
            if (begin <= p && p < end)
                return &arena;
        }
        return nullptr;
    }

    void init_free_list(arena_t &arena)
    {
        auto       begin = arena.get();
        auto const end   = begin + slots_per_arena_;
        while (begin != end)
        {
            ASAN_UNPOISON_MEMORY_REGION(begin, sizeof(slot_type_t));
            *reinterpret_cast<free_list_value_t *>(begin) = begin + 1;
            ASAN_POISON_MEMORY_REGION(begin, sizeof(slot_type_t));
            ++begin;
        }
        free_list_value_t *tail = reinterpret_cast<free_list_value_t *>(end - 1);
        ASAN_UNPOISON_MEMORY_REGION(tail, sizeof(slot_type_t));
        *tail = nullptr;
        ASAN_POISON_MEMORY_REGION(tail, sizeof(slot_type_t));
    }

    void init_free_list()
    {
        for (std::size_t i = 0; i < arenas_.size(); ++i)
        {
            auto &curr_arena = arenas_[i];
            init_free_list(curr_arena);

            free_list_value_t *tail = reinterpret_cast<free_list_value_t *>(curr_arena.get() + slots_per_arena_ - 1);
            ASAN_UNPOISON_MEMORY_REGION(tail, sizeof(slot_type_t));
            if (i + 1 < arenas_.size())
                *tail = arenas_[i + 1].get();
            else
                *tail = nullptr;
            ASAN_POISON_MEMORY_REGION(tail, sizeof(slot_type_t));
        }

        if (!arenas_.empty())
            free_list_head_ = arenas_.front().get();
    }

    std::vector<arena_t> arenas_;
    free_list_value_t    free_list_head_{};
    std::uint32_t        slots_per_arena_{};

#undef ASAN_UNPOISON_MEMORY_REGION
#undef ASAN_POISON_MEMORY_REGION
};
} // namespace aegis::allocators
