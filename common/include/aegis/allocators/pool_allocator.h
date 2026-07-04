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

    void *acquire()
    {
        if (free_list_head_)
        {
            auto slot       = free_list_head_;
            free_list_head_ = *reinterpret_cast<free_list_value_t *>(free_list_head_);
            return slot;
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
            return;
        }

        free_list_value_t p_slot    = p;
        free_list_value_t prev_slot = nullptr;
        free_list_value_t curr_slot = free_list_head_;
        while (curr_slot)
        {
            free_list_value_t next_slot = *reinterpret_cast<free_list_value_t *>(curr_slot);
            // Double-free prevention
            if (p_slot == curr_slot || p_slot == next_slot)
                return;

            if (p_slot < curr_slot)
            {
                if (prev_slot)
                    *reinterpret_cast<free_list_value_t *>(prev_slot) = p_slot;
                else
                    free_list_head_ = p_slot;
                *reinterpret_cast<free_list_value_t *>(p_slot) = curr_slot;
                return;
            }
            else if (curr_slot < p_slot && p_slot < next_slot)
            {
                *reinterpret_cast<free_list_value_t *>(curr_slot) = p_slot;
                *reinterpret_cast<free_list_value_t *>(p_slot)    = next_slot;
                return;
            }
            else if (!next_slot)
            {
                *reinterpret_cast<free_list_value_t *>(curr_slot) = p_slot;
                *reinterpret_cast<free_list_value_t *>(p_slot)    = nullptr;
                return;
            }

            prev_slot = curr_slot;
            curr_slot = next_slot;
        }
    }

  private:
    arena_t make_arena()
    {
        void *mem = ::operator new(sizeof(slot_type_t) * slots_per_arena_, slot_alignment);
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
            *reinterpret_cast<free_list_value_t *>(begin) = begin + 1;
            ++begin;
        }
    }

    void init_free_list()
    {
        for (std::size_t i = 0; i < arenas_.size(); ++i)
        {
            auto &curr_arena = arenas_[i];
            init_free_list(curr_arena);

            auto const end = curr_arena.get() + slots_per_arena_;
            if (i + 1 < arenas_.size())
            {
                *reinterpret_cast<free_list_value_t *>(end - 1) = arenas_[i + 1].get();
            }
            else
            {
                *reinterpret_cast<free_list_value_t *>(end - 1) = nullptr;
            }
        }

        if (!arenas_.empty())
            free_list_head_ = arenas_.front().get();
    }

    std::vector<arena_t> arenas_;
    free_list_value_t    free_list_head_{};
    std::uint32_t        slots_per_arena_{};
};
} // namespace aegis::allocators
