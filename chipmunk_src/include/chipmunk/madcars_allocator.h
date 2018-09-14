//
// Created by lookuut on 13.09.18.
//

#include <stdio.h>
#include <stdlib.h>

#define HEAP_MAX_SIZE 1024 * 1024


struct madcarsPointer {
    void * mem_pointer;
    long size;
};

typedef struct madcarsPointer madcarsPointer;

struct madcarsAllocator {
    long allocated_memory_pos;
    void * heap;
    void * pointers;
};

typedef struct madcarsAllocator madcarsAllocator;


madcarsAllocator mad_cars_heap = {0l, NULL};

void * madcars_allocator(size_t nitems, size_t size) {
    extern madcarsAllocator mad_cars_heap;

    if (mad_cars_heap.heap == NULL) {
        mad_cars_heap.heap = calloc(8, HEAP_MAX_SIZE);
    }
    void * mem = (void*) ((char*)mad_cars_heap.heap + mad_cars_heap.allocated_memory_pos + 1);
    mad_cars_heap.allocated_memory_pos += size * nitems;

    return mem;
}

void my_free(void * ptr) {

}

void * madcars_reallocator(void * ptr, size_t nitems, size_t size) {
    my_free(ptr);

    return madcars_allocator(nitems, size);
}
