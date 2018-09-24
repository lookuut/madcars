//
// Created by lookuut on 13.09.18.
//

#include <stdio.h>
#include <stdlib.h>

#ifndef MADCARS_ACCLOCATOR_H
#define MADCARS_ACCLOCATOR_H

#define HEAP_BUFF_SIZE_IN_BYTES 1024 * 1024
#define BUFF_BLOCK_SIZE 4

struct madcars_allocator {
    long size;
    long allocated_memory_pos;
    void * heap;
};
typedef struct madcars_allocator madcars_allocator;


void * madcars_alloc(size_t nitems, size_t size);
void madcars_free(void * ptr);
void * madcars_realloc(void *ptr, size_t nitems);
madcars_allocator * madcars_get_heap();

#endif