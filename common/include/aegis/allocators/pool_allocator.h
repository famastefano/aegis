#pragma once

#include <algorithm>
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

namespace impl
{
template <typename T> struct PoolAllocatorTraits
{
    using free_slot_t = unsigned char *;
    using slot_type_t = typename std::conditional_t<sizeof(free_slot_t) >= sizeof(T), free_slot_t, T>;

    struct arena_deleter_t
    {
        void operator()(void *p)
        { ::operator delete[](p, std::align_val_t(alignment())); }
    };
    using arena_t = typename std::unique_ptr<unsigned char[], arena_deleter_t>;

    static constexpr std::size_t alignment()
    { return std::max(alignof(T), alignof(free_slot_t)); }

    static constexpr std::size_t slot_size()
    {
        if constexpr (sizeof(slot_type_t) % alignment() == 0)
            return sizeof(slot_type_t);
        else
            return sizeof(slot_type_t) + alignment();
    }

    static constexpr std::size_t alignment_stride()
    {
        constexpr auto sz     = slot_size();
        constexpr auto align  = alignment();
        constexpr auto offset = sz & (align - 1);
        return sz - offset;
    }

    static constexpr bool is_aligned(free_slot_t slot)
    { return !(reinterpret_cast<std::uintptr_t>(slot) & (alignment() - 1)); }

    static constexpr free_slot_t align_slot(free_slot_t slot)
    {
        constexpr auto align = alignment();
        constexpr auto sz    = slot_size();

        void  *p       = slot;
        size_t buffer  = sz;
        void  *aligned = std::align(align, sizeof(slot_type_t), p, buffer);
        return reinterpret_cast<free_slot_t>(aligned);
    }

    static constexpr free_slot_t from_pos(arena_t const &arena, std::size_t pos)
    { return align_slot(arena.get() + slot_size() * pos); }

    static constexpr free_slot_t next(free_slot_t slot)
    { return *as_ptr(slot); }

    static constexpr free_slot_t *as_ptr(free_slot_t slot)
    { return std::launder(reinterpret_cast<free_slot_t *>(slot)); }
};
} // namespace impl

/*
 * Pool allocator for type T.
 * Memory layout is a collection of equally sized arenas.
 * An in-place free list is used, so the smallest block size will be sizeof(void*).
 */
template <typename T> class PoolAllocator
{
    using traits_t = typename impl::PoolAllocatorTraits<T>;

  public:
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
            ASAN_UNPOISON_MEMORY_REGION(free_list_head_, traits_t::slot_size());
            auto slot       = free_list_head_;
            free_list_head_ = *traits_t::as_ptr(free_list_head_);
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
        if (!p || !traits_t::is_aligned(reinterpret_cast<typename traits_t::free_slot_t>(p)) || !find_arena(p))
            return;

        if (!free_list_head_)
        {
            free_list_head_                    = reinterpret_cast<typename traits_t::free_slot_t>(p);
            *traits_t::as_ptr(free_list_head_) = nullptr;
            ASAN_POISON_MEMORY_REGION(free_list_head_, traits_t::slot_size());
            return;
        }

        typename traits_t::free_slot_t p_slot    = reinterpret_cast<typename traits_t::free_slot_t>(p);
        typename traits_t::free_slot_t prev_slot = nullptr;
        typename traits_t::free_slot_t curr_slot = free_list_head_;
        while (curr_slot)
        {
            ASAN_UNPOISON_MEMORY_REGION(curr_slot, traits_t::slot_size());
            auto next_slot = traits_t::next(curr_slot);
            // Double-free prevention
            if (p_slot == curr_slot || p_slot == next_slot)
            {
                ASAN_POISON_MEMORY_REGION(curr_slot, traits_t::slot_size());
                return;
            }

            if (p_slot < curr_slot)
            {
                if (prev_slot)
                {
                    ASAN_UNPOISON_MEMORY_REGION(prev_slot, traits_t::slot_size());
                    *traits_t::as_ptr(prev_slot) = p_slot;
                    ASAN_POISON_MEMORY_REGION(prev_slot, traits_t::slot_size());
                }
                else
                {
                    free_list_head_ = p_slot;
                }
                ASAN_UNPOISON_MEMORY_REGION(p_slot, traits_t::slot_size());
                *traits_t::as_ptr(p_slot) = curr_slot;
                ASAN_POISON_MEMORY_REGION(p_slot, traits_t::slot_size());
                return;
            }
            else if (curr_slot < p_slot && p_slot < next_slot)
            {
                ASAN_UNPOISON_MEMORY_REGION(curr_slot, traits_t::slot_size());
                ASAN_UNPOISON_MEMORY_REGION(p_slot, traits_t::slot_size());
                *traits_t::as_ptr(curr_slot) = p_slot;
                *traits_t::as_ptr(p_slot)    = next_slot;
                ASAN_POISON_MEMORY_REGION(curr_slot, traits_t::slot_size());
                ASAN_POISON_MEMORY_REGION(p_slot, traits_t::slot_size());
                return;
            }
            else if (!next_slot)
            {
                ASAN_UNPOISON_MEMORY_REGION(curr_slot, traits_t::slot_size());
                ASAN_UNPOISON_MEMORY_REGION(p_slot, traits_t::slot_size());
                *traits_t::as_ptr(curr_slot) = p_slot;
                *traits_t::as_ptr(p_slot)    = nullptr;
                ASAN_POISON_MEMORY_REGION(curr_slot, traits_t::slot_size());
                ASAN_POISON_MEMORY_REGION(p_slot, traits_t::slot_size());
                return;
            }
            ASAN_POISON_MEMORY_REGION(curr_slot, traits_t::slot_size());

            prev_slot = curr_slot;
            curr_slot = next_slot;
        }
    }

#ifdef AEGIS_BUILD_TEST
    bool debug_validate() const
    {
        std::unordered_set<typename traits_t::free_slot_t> traversed_slots;

        auto slot = free_list_head_;
        while (slot)
        {
            auto const slot_address = reinterpret_cast<std::uintptr_t>(slot);
            auto const alignment    = static_cast<std::size_t>(traits_t::alignment());
            if (slot_address & (alignment - 1ull))
                return false;

            if (!find_arena(slot))
                return false;

            if (traversed_slots.contains(slot))
                return false;

            traversed_slots.emplace_hint(traversed_slots.end(), slot);

            ASAN_UNPOISON_MEMORY_REGION(slot, traits_t::slot_size());
            slot = *traits_t::as_ptr(slot);
            ASAN_POISON_MEMORY_REGION(slot, traits_t::slot_size());
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

        auto slot = free_list_head_;
        if (slot)
            ++free_slots;
        while (slot)
        {
            ASAN_UNPOISON_MEMORY_REGION(slot, traits_t::slot_size());
            slot = *traits_t::as_ptr(slot);
            ASAN_POISON_MEMORY_REGION(slot, traits_t::slot_size());
            ++free_slots;
        }
        return 1.0f - float(free_slots) / allocated_slots;
    }
#endif

  private:
    typename traits_t::arena_t make_arena()
    {
        auto const arena_size = traits_t::slot_size() * slots_per_arena_;
        void      *mem        = ::operator new[](arena_size, std::align_val_t(traits_t::alignment()));
        ASAN_POISON_MEMORY_REGION(mem, arena_size);
        return typename traits_t::arena_t(static_cast<unsigned char *>(mem));
    }

    typename traits_t::arena_t const *find_arena(void *p) const
    {
        auto const p_addr = reinterpret_cast<std::uintptr_t>(p);
        for (auto const &arena : arenas_)
        {
            auto const begin = reinterpret_cast<std::uintptr_t>(traits_t::from_pos(arena, 0));
            auto const end   = reinterpret_cast<std::uintptr_t>(traits_t::from_pos(arena, slots_per_arena_));
            if (begin <= p_addr && p_addr < end)
                return &arena;
        }
        return nullptr;
    }

    void init_free_list(typename traits_t::arena_t &arena)
    {
        for (std::size_t slot_num = 0; slot_num < slots_per_arena_; ++slot_num)
        {
            auto slot = traits_t::from_pos(arena, slot_num);
            ASAN_UNPOISON_MEMORY_REGION(slot, traits_t::slot_size());
            auto next               = traits_t::from_pos(arena, slot_num + 1);
            *traits_t::as_ptr(slot) = next;
            ASAN_POISON_MEMORY_REGION(slot, traits_t::slot_size());
        }
        auto tail = traits_t::as_ptr(traits_t::from_pos(arena, slots_per_arena_ - 1));
        ASAN_UNPOISON_MEMORY_REGION(tail, traits_t::slot_size());
        *tail = nullptr;
        ASAN_POISON_MEMORY_REGION(tail, traits_t::slot_size());
    }

    void init_free_list()
    {
        for (std::size_t i = 0; i < arenas_.size(); ++i)
        {
            auto &curr_arena = arenas_[i];
            init_free_list(curr_arena);

            auto tail = traits_t::as_ptr(traits_t::from_pos(curr_arena, slots_per_arena_ - 1));
            ASAN_UNPOISON_MEMORY_REGION(tail, traits_t::slot_size());
            if (i + 1 < arenas_.size())
                *tail = traits_t::from_pos(arenas_[i + 1], 0);
            else
                *tail = nullptr;
            ASAN_POISON_MEMORY_REGION(tail, traits_t::slot_size());
        }

        if (!arenas_.empty())
            free_list_head_ = traits_t::from_pos(arenas_.front(), 0);
    }

    std::vector<typename traits_t::arena_t> arenas_;
    typename traits_t::free_slot_t          free_list_head_{};
    std::uint32_t                           slots_per_arena_{};

#undef ASAN_UNPOISON_MEMORY_REGION
#undef ASAN_POISON_MEMORY_REGION
};
} // namespace aegis::allocators
