//
// Created by lookuut on 13.09.18.
//

#include <assert.h>
#include <chipmunk/chipmunk.h>
#include <memory.h>
#include "madcars_allocator.h"

madcarsAllocator mad_cars_heap = {0l, 0l, NULL};

void * madcars_allocator(size_t nitems, size_t size) {
    extern madcarsAllocator mad_cars_heap;

    long mem_size = size * nitems;

    if (mad_cars_heap.allocated_memory_pos +  mem_size > mad_cars_heap.size) {

        mad_cars_heap.heap = realloc(mad_cars_heap.heap , mad_cars_heap.size + BUFF_BLOCK_SIZE * HEAP_BUFF_SIZE_IN_BYTES);
        cpAssertHard(mad_cars_heap.heap, "No memory");

        mad_cars_heap.size += BUFF_BLOCK_SIZE * HEAP_BUFF_SIZE_IN_BYTES;
    }

    void * mem = (void*) ((char*)mad_cars_heap.heap + mad_cars_heap.allocated_memory_pos);
    mad_cars_heap.allocated_memory_pos += mem_size;

    return mem;
}

void madcars_free(void * ptr) {
}

void * madcars_reallocator(void * ptr, size_t nitems) {

    void * new_mem = madcars_allocator(nitems, 1);

    memcpy(new_mem, ptr, nitems);

    return new_mem;
}

madcarsAllocator * madcars_get_heap() {
    extern madcarsAllocator mad_cars_heap;
    return &mad_cars_heap;
}
