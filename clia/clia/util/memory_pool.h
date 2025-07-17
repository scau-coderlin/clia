#ifndef CLIA_BASE_MEMORY_POOL_H_
#define CLIA_BASE_MEMORY_POOL_H_

namespace clia {
    namespace util {
        template <int Nm, int Sz>
        class MemoryPool {
        public:
            MemoryPool();
            ~MemoryPool();

            void* allocate() {
                // Allocate memory from the pool
                return nullptr; // Placeholder
            }

            void deallocate(void* ptr) {
                // Deallocate memory back to the pool
            }
        }; 
    }
}

#endif