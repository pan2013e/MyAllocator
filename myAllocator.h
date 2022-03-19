#ifndef _MY_ALLOC_
#define _MY_ALLOC_

#include <type_traits>

// size of pointer: 8
#define _ALIGN sizeof(void*)
// round up to multiple of 8
#define ROUND_UP(x) (((x)+(_ALIGN)-1)&~((_ALIGN)-1))

namespace myAlloc {
    template<class T, size_t BlockSize = 4096>
    class allocator {
    public:
        typedef T value_type; // required in vector interface
        template<class U> struct rebind {
            typedef allocator<U> other;
        };
        T* allocate(size_t n) noexcept {
            // large memory request, call operator new directly
            if (n > BlockSize) {
                return static_cast<T*>(::operator new((n * ROUND_UP(sizeof(T)))));
            }
            cnt += n;
            T* ret;
            // blocks available
            if (blocks >= n + cnt && free_list) {
                // allocate block & move head ptr
                ret = reinterpret_cast<T*>(free_list);
                free_list->next = reinterpret_cast<block*>(reinterpret_cast<char*>(free_list) + ROUND_UP(sizeof(T)) * n);
                free_list = free_list->next;
            } else { // not enough
                ret = allocate(ROUND_UP(sizeof(T)), n);
            }
            return ret;
        }
        void deallocate(T* p, size_t n) noexcept {
            // large memory request, call operator delete directly
            if (n > BlockSize) {
                ::operator delete(p);
                return;
            }
            // recycle block & move head ptr
            block* start = reinterpret_cast<block*>(p), * temp = start;
            block* end = reinterpret_cast<block*>(reinterpret_cast<char*>(temp) + ROUND_UP(sizeof(T)) * (n - 1));
            while (temp != end) {
                temp->next = reinterpret_cast<block*>(reinterpret_cast<char*>(temp) + ROUND_UP(sizeof(T)));
                temp = temp->next;
            }
            end->next = recycle;
            recycle = start;
        }

    private:
        // overloaded internal allocate, allocates chunks
        T* allocate(size_t size, size_t offset, size_t blk_sz = BlockSize) {
            block* p, * start, * temp = free_list, * ret;
            size_t chunk = blk_sz * size;
            // request memory
            start = p = reinterpret_cast<block*>(::operator new(chunk));
            blocks += blk_sz;
            // create linkage
            for (int i = 0;i < blk_sz - 1;i++) {
                p->next = reinterpret_cast<block*>(reinterpret_cast<char*>(p) + size);
                p = p->next;
            }
            p->next = nullptr;
            // link to free_list
            if (!free_list) {
                free_list = start;
            }else {
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = start;
            }
            // move head ptr
            ret = free_list;
            free_list->next = reinterpret_cast<block*>(reinterpret_cast<char*>(free_list) + size * offset);
            free_list = free_list->next;
            return reinterpret_cast<T*>(ret);
        }
        struct block { // store ptrs/data
            block* next;
        };
        block* free_list = nullptr;
        block* recycle = nullptr; // head of deallocated memory blocks
        size_t cnt = 0;    // count used blocks
        size_t blocks = 0; // count total allocated blocks
        static_assert(BlockSize > 0, "BlockSize should be positive");
    };
};
#endif // _MY_ALLOC_