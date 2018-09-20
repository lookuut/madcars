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

class Simulation {
private:

    Match * match;
    madcarsAllocator * heap;

    list<md_heap_state> copies;

    short step;

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

public:

    static bool deleteAll( md_heap_state theElement ) { free(theElement.heap); return true; }


    vector<int> simulation_step_sizes;

    Simulation() {
        this->heap = this->get_heap();

        simulation_step_sizes.push_back(2);
        simulation_step_sizes.push_back(4);
        simulation_step_sizes.push_back(8);
        simulation_step_sizes.push_back(32);
        simulation_step_sizes.push_back(64);
    }

    void fix_heap() {

        if (this->copies.size() > 0) {
            this->copies.remove_if(deleteAll);
        }

        this->copies = list<md_heap_state>();
    }

    list<short> win_definer(short my_command, short winner, Match *match, int tick) {

        Player * my_player = match->get_my_player();
        Player * enemy_player = match->get_enemy_player();

        list<short> win_steps;

        for (short step = 0; step <= Player::max_command; step++) {
            copy_heap();

            my_player->press_command(my_command);
            enemy_player->press_command(step);

            match->tick(tick);
            cpSpaceStep(match->get_space(), Constants::SPACE_TICK);
            copy_heap();

            cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

            if ((winner == 1 && enemy_player->is_dead()) || (winner == 2 && my_player->is_dead()) && (winner == 0 && my_player->is_dead() && enemy_player->is_dead()) ) {
                win_steps.push_back(step);
            }

            my_player->set_dead(false);
            enemy_player->set_dead(false);
            restore_heap();
            restore_heap();
        }

        return win_steps;
    }

    short enemy_step_definer(short my_prev_command, CarState *my_state, CarState *enemy_state, Match *match, int tick, int round) {

        Player * my_player = match->get_my_player();
        Player * enemy_player = match->get_enemy_player();

        for (short step = 0; step <= Player::max_command; step++) {
            copy_heap();

            my_player->press_command(my_prev_command);
            enemy_player->press_command(step);

            match->tick(tick);

            cpSpaceStep(match->get_space(), Constants::SPACE_TICK);
            copy_heap();

            cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

            if (my_state->is_equal(my_player->get_car()) && enemy_state->is_equal(enemy_player->get_car())) {
                restore_heap();
                fix_heap();

                return step;
            }

            restore_heap();
            restore_heap();
        }

        throw std::invalid_argument( "Cant find any step" );
    }

    void run(
            Match * match,
            Player * my_player,
            Player * enemy_player,
            list<short> steps,
            SimVariance ** variant,
            int tick,
            int deep,
            list<short> * enemy_steps,
            list<short>::iterator &enemy_step_pos
            ) {

        bool win = false;

        for (short command = 0; command <= Constants::STEP_MAX_SIZE; command++) {
            copy_heap();

            my_player->set_dead(false);
            enemy_player->set_dead(false);

            steps.push_back(command);

            list<short>::iterator enemy_steps_it = enemy_steps->begin();


            for (int i = 0; i <= simulation_step_sizes[deep]; i++) {
                my_player->push_command(command);

                short enemy_step = (enemy_step_pos != enemy_steps->end() ? *enemy_step_pos : Constants::default_step);
                enemy_player->push_command(enemy_step);

                ++enemy_step_pos;
            }

            size_t f_size = my_player->command_queue_size();
            size_t s_size = enemy_player->command_queue_size();

            int step = 0;
            for(int command_number = 0; command_number < std::max(s_size, f_size); command_number++, step++) {

                match->tick(tick + step);

                cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

                if (enemy_player->is_dead() && !my_player->is_dead()) {/// find situation where we win
                    win = true;

                    if (*variant == NULL) {
                        *variant = new SimVariance(new CarState(my_player->get_car()), new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes);
                    } else if ((*variant)->is_weakness(tick + step, my_player->get_car(), enemy_player->get_car()))  {
                        delete *variant;
                        *variant = new SimVariance(new CarState(my_player->get_car()), new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes);
                    }

                    break;
                } else if (my_player->is_dead()) {
                    break;
                }
            }

            my_player->clear_command_queue();
            enemy_player->clear_command_queue();

            if (!win && deep + 1 < simulation_step_sizes.size() && !my_player->is_dead()) {
                run(match, my_player, enemy_player, steps, variant, tick + step, deep + 1, enemy_steps, enemy_step_pos);
            } else if (!win && !my_player->is_dead() && deep + 1 >= simulation_step_sizes.size()) {//cant go deeper

                if (*variant == NULL) {
                    *variant = new SimVariance(new CarState(my_player->get_car()), new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes);
                } else {
                    if ((*variant)->is_weakness(tick + step, my_player->get_car(), enemy_player->get_car()) ) {
                        delete *variant;
                        *variant = new SimVariance(new CarState(my_player->get_car()),  new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes);
                    }
                }
            }

            my_player->set_dead(false);
            enemy_player->set_dead(false);
            match->clear_dead_players();
            steps.pop_back();
            restore_heap();
        }
    }
};


#endif //MADCARS_SIMULATION_H
