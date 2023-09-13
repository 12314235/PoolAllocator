#pragma once
#include <cstddef>
#include <list>
#include <vector>
#include <stdexcept>

struct Chunk{
    Chunk* next = nullptr;

    Chunk(const Chunk& other) : next(other.next){}
    Chunk() = default;
    ~Chunk() = default;
};

class Pool{
public:

    size_t chunk_size;
    size_t chunk_count;

    size_t allocated_chunks_count = 0;

    Chunk* memory_start = nullptr;

    Chunk* free_chunks = nullptr;

    Pool() = default;
    Pool(size_t chunk_size, size_t chunk_count, size_t value_size) : chunk_size(chunk_size * value_size), chunk_count(chunk_count){
        memory_start = reinterpret_cast<Chunk*>(malloc(chunk_size * chunk_count * value_size));
        free_chunks = memory_start;
        auto current_chunk = free_chunks;
        for(int i = 0; i < chunk_count - 1; ++i) {
            current_chunk->next = reinterpret_cast<Chunk*>(reinterpret_cast<char *>(current_chunk) + chunk_size * value_size);
            current_chunk = current_chunk->next;
        }
        current_chunk->next = nullptr;
    }
    ~Pool() = default;
    Pool(const Pool& other) : chunk_size(other.chunk_size), chunk_count(other.chunk_count){}
};

template <typename Value_Type>
class pool_allocator{

public:
    using size_type = size_t;
    using value_type = Value_Type;
    using pointer = Value_Type*;
    using const_pointer = const Value_Type*;
    using reference = Value_Type&;
    using const_reference = const Value_Type&;
    using difference_type = std::make_signed_t<size_type>;
    using pair = std::pair<size_t, size_t>;

public:

    std::list<Pool*> pools;

public:

    bool operator==(const pool_allocator<Value_Type>& other) {
        return pools == other.pools;
    }

    bool operator!=(const pool_allocator<Value_Type>& other){
        return *this != other;
    }

    template<typename U>
    pool_allocator(const pool_allocator<U>& other) {
        pools = other.pools;
    }

    pool_allocator(const std::vector<std::pair<size_t, size_t>>& init) {
        for(auto i : init) {
            auto new_pool = new Pool(i.first, i.second, sizeof(Value_Type));
            this->pools.push_back(new_pool);
        }
    }

    pool_allocator(const pool_allocator<Value_Type>& other) : pools(other.pools) {

    }

    pointer allocate(size_t n) {
        Chunk* free_chunk = nullptr;
        for(auto i : pools) {
            if (n * sizeof(Value_Type) <= i->chunk_size && i->chunk_count - i->allocated_chunks_count > 0 && i->free_chunks != nullptr) {
                i->allocated_chunks_count++;
                free_chunk = i->free_chunks;
                i->free_chunks = i->free_chunks->next;

                break;
            }
        }

        if(free_chunk == nullptr) {
            throw std::bad_alloc();
        }

        return reinterpret_cast<pointer>(free_chunk);
    }

    void deallocate(value_type* p, size_t item_count) {
        auto chunk_to_delete = reinterpret_cast<Chunk*>(p);
        Pool* pool = nullptr;
        for(auto i : pools) {
            if(chunk_to_delete >= i->memory_start && chunk_to_delete <= i->memory_start + i->chunk_size * i->chunk_count * sizeof(Value_Type)){
                pool = i;
                break;
            }
        }

        if(pool == nullptr ) {
            throw std::runtime_error("...");
        }

        size_t n = item_count % pool->chunk_size == 0 ? item_count / pool->chunk_size : item_count / pool->chunk_size + 1;

        auto current_chunk = chunk_to_delete - (n-1) * pool->chunk_size;
        pool->free_chunks = current_chunk;

        for(int i = 0; i < n; i++){
            current_chunk->next = reinterpret_cast<Chunk*>(reinterpret_cast<char *>(current_chunk) + pool->chunk_size * sizeof(Value_Type));
            current_chunk = current_chunk->next;
        }

        pool->allocated_chunks_count -= n;

        pool->free_chunks = chunk_to_delete;
    }
};

