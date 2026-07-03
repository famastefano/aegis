#pragma once

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace aegis::allocators
{
/*
 * Pool allocator for type T.
 * Memory layout is a collection of equally sized arenas.
 * An in-place free list is used, so the smallest block size will be sizeof(void*).
 */
template <typename T> class PoolAllocator
{
    using slot_type_t = typename std::conditional_t<sizeof(void *) >= sizeof(T), void *, T>;
    struct arena_deleter_t
    {
        void operator()(slot_type_t *p)
        { ::operator delete[](p, std::align_val_t{alignof(slot_type_t)}); }
    };
    using arena_t = typename std::unique_ptr<slot_type_t[], arena_deleter_t>;

  public:
    PoolAllocator(std::uint32_t slots_per_arena, std::uint32_t prellocated_arenas_count = 0)
        : arenas_(prellocated_arenas_count), slots_per_arena_(slots_per_arena)
    {
        for (auto &arena : arenas_)
            arena = make_arena();

        init_free_list();
    }

    void *acquire()
    {
        if (free_list_head_)
        {
            void *slot      = *free_list_head_;
            free_list_head_ = reinterpret_cast<void **>(*free_list_head_);
            return slot;
        }
        else
        {
            auto &arena = arenas_.emplace_back(make_arena());
            init_free_list(arena);
            free_list_head_ = reinterpret_cast<void **>(arena.get());
            return *free_list_head_;
        }
    }

    void release(T *p)
    {
        auto *arena = find_arena(p);
        if (!arena)
            return;

        if (free_list_head_)
        {
            auto *slot = free_list_head_;
            while (slot)
            {
                void *next_slot_addr = *slot;
                if (next_slot_addr > static_cast<void *>(p) || !next_slot_addr)
                {
                    *slot                         = reinterpret_cast<void **>(p);
                    *reinterpret_cast<void **>(p) = next_slot_addr;
                    return;
                }
                slot = reinterpret_cast<void **>(next_slot_addr);
            }
        }
        else
        {
            free_list_head_ = reinterpret_cast<void **>(p);
        }
    }

  private:
    arena_t make_arena()
    {
        void *mem = ::operator new[](sizeof(slot_type_t) * slots_per_arena_, std::align_val_t{alignof(slot_type_t)});
        return arena_t(static_cast<slot_type_t *>(mem));
    }

    arena_t *find_arena(void *p)
    {
        for (auto &arena : arenas_)
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
        auto       begin = static_cast<slot_type_t *>(arena.get());
        auto const end   = begin + slots_per_arena_;
        while (begin != end)
        {
            *reinterpret_cast<void **>(begin) = static_cast<void *>(begin + 1);
            ++begin;
        }
    }

    void init_free_list()
    {
        for (std::size_t i = 0; i < arenas_.size(); ++i)
        {
            auto &curr_arena = arenas_[i];
            init_free_list(curr_arena);

            auto const end = static_cast<slot_type_t *>(curr_arena.get()) + slots_per_arena_;
            if (i + 1 < arenas_.size())
                *reinterpret_cast<void **>(end - 1) = static_cast<void *>(arenas_[i + 1].get());
            else
                *reinterpret_cast<void **>(end - 1) = nullptr;
        }
        if (!arenas_.empty())
        {
            free_list_head_ = reinterpret_cast<void **>(arenas_.front().get());
        }
    }

    std::vector<arena_t> arenas_;
    void               **free_list_head_{};
    std::uint32_t        slots_per_arena_;
};
} // namespace aegis::allocators
