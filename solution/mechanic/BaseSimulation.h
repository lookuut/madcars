//
// Created by lookuut on 13.09.18.
//

#ifndef MADCARS_SIMULATION_H
#define MADCARS_SIMULATION_H


#include <chipmunk/chipmunk_structs.h>
#include "Match.h"
#include "CarState.h"
#include <madcars_allocator.h>
#include "SimVariance.h"
#include <algorithm>    // std::max

struct md_heap_state {
    void * heap;
    size_t size;
};

typedef struct md_heap_state md_heap_state;

class BaseSimulation {
private:

    madcarsAllocator * heap;

    list<md_heap_state> copies;

    madcarsAllocator * get_heap() {
        extern madcarsAllocator mad_cars_heap;
        return &mad_cars_heap;
    }

protected:

    void copy_heap() {
        void * heap_copy = calloc(1, this->heap->allocated_memory_pos);
        memcpy(heap_copy, this->heap->heap, this->heap->allocated_memory_pos);

        md_heap_state heap_state;
        heap_state.heap = heap_copy;
        heap_state.size = this->heap->allocated_memory_pos;

        copies.push_back(heap_state);
    }

    void restore_heap() {
        md_heap_state md_heap_state = copies.back();

        memcpy(this->heap->heap, md_heap_state.heap, md_heap_state.size);

        if (md_heap_state.size < this->heap->allocated_memory_pos) {
            memset((void*) ((char*)this->heap->heap + md_heap_state.size), 0, this->heap->allocated_memory_pos - md_heap_state.size);
        }

        this->heap->allocated_memory_pos = md_heap_state.size;

        free(md_heap_state.heap);

        copies.pop_back();
    }


    void fix_heap() {

        if (this->copies.size() > 0) {
            this->copies.remove_if(deleteAllHeap);
        }

        this->copies = list<md_heap_state>();
    }

    vector<int> simulation_step_sizes;

public:


    BaseSimulation() {
        this->heap = this->get_heap();

        simulation_step_sizes.push_back(2);
        simulation_step_sizes.push_back(4);
        simulation_step_sizes.push_back(8);
        simulation_step_sizes.push_back(32);
        simulation_step_sizes.push_back(64);
    }

    static bool deleteAllHeap( md_heap_state theElement ) { free(theElement.heap); return true; }
    static bool deleteAll( CarState * theElement ) { free(theElement); return true; }

    void set_step_sizes(vector<int> simulation_step_sizes) {
        this->simulation_step_sizes = simulation_step_sizes;
    }

    list<short> win_definer(short my_command, short winner, Match *match, int tick);

    short enemy_step_definer(short my_prev_command, CarState *my_state, CarState *enemy_state, Match *match, int tick, int round);

    int check_future_steps (list<short> * steps, list<short> * enemy_steps, list<CarState*> *states, Match * match, int tick);
};


#endif //MADCARS_SIMULATION_H
