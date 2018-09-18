//
// Created by lookuut on 13.09.18.
//

#include <stdio.h>
#include <stdlib.h>

#ifndef MADCARS_ACCLOCATOR_H
#define MADCARS_ACCLOCATOR_H

#define HEAP_BUFF_SIZE_IN_BYTES 1024 * 1024
#define BUFF_BLOCK_SIZE 4

struct madcarsAllocator {
    long size;
    long allocated_memory_pos;
    void * heap;
};
typedef struct madcarsAllocator madcarsAllocator;


void * madcars_allocator(size_t nitems, size_t size);
void madcars_free(void * ptr);
void * madcars_reallocator(void * ptr, size_t nitems);
madcarsAllocator * madcars_get_heap();

#endif