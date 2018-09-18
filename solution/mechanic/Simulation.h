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

class Simulation {
private:

    Match * match;
    madcarsAllocator * heap;

    list<void *> copies;

    short step;

    madcarsAllocator * get_heap() {
        extern madcarsAllocator mad_cars_heap;
        return &mad_cars_heap;
    }

protected:

    void copy_heap() {
        void * heap_copy = calloc(1, this->heap->size);
        memcpy(heap_copy, this->heap->heap, this->heap->size);
        copies.push_back(heap_copy);
    }

    void restore_heap() {
        void * heap_copy = copies.back();

        memcpy(this->heap->heap, heap_copy, this->heap->size); // memory leak when size changed
        free(heap_copy);
        copies.pop_back();
    }

public:

    static bool deleteAll( void * theElement ) { free(theElement); return true; }

    Simulation() {
        this->heap = this->get_heap();
    }

    void fix_heap() {

        if (this->copies.size() > 0) {
            this->copies.remove_if(deleteAll);
        }

        this->copies = list<void*>();
    }

    list<short> * synchronizedStates(list<short> *my_steps, list<CarState *> *enemy_states,
                                     list<short>::iterator &cur_step, list<CarState *>::iterator &cur_enemy_state,
                                     Match *match, int tick, list<CarState *> *my_states, list<CarState *>::iterator &cur_my_state) {

        list<short> * good_steps = new list<short>;

        for (short step = 0; step <= Player::max_command; step++) {
            copy_heap();

            Player * my_player = match->get_my_player();
            my_player->push_command(*cur_step);

            Player * enemy_player = match->get_enemy_player();
            enemy_player->push_command(step);

            match->tick(tick);
            cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

            if ((*cur_my_state)->is_equal(my_player->get_car()) && (*cur_enemy_state)->is_equal(enemy_player->get_car())) {// Good sima

                if (std::next(cur_enemy_state) == enemy_states->end()) {//only one way found, okey thats good, then return
                    restore_heap();
                    fix_heap();

                    return good_steps;
                }

                good_steps->push_back(step);

                if (std::next(cur_enemy_state) != enemy_states->end()) {
                    list<short> * result = synchronizedStates(my_steps, enemy_states, ++cur_step, ++cur_enemy_state,
                                                              match, tick + 1, my_states, ++cur_my_state);
                    if (result != NULL) {
                        result->push_front(step);
                        return result;
                    }
                }
            }

            restore_heap();
        }

        if (my_steps->begin() != cur_step) {
            --cur_step;
        }

        if (enemy_states->begin() != cur_enemy_state) {
            --cur_enemy_state;
        }

        if (my_states->begin() != cur_my_state) {
            --cur_my_state;
        }


        delete good_steps;
        return NULL;
    }

    void run(Match * match, Player * my_player, Player * enemy_player, list<short> steps, SimVariance ** variant, int tick, int deep, list<short> * enemy_steps) {

        bool win = false;

        for (short command = 0; command <= Player::max_command; command++) {
            copy_heap();

            my_player->set_dead(false);
            enemy_player->set_dead(false);

            steps.push_back(command);

            list<short>::iterator enemy_steps_it = enemy_steps->begin();
            for (int step = 0; step < Constants::SIMULATION_STEP_SIZE; step++) {
                my_player->push_command(command);

                short enemy_step = (enemy_steps_it != enemy_steps->end() ? *enemy_steps_it : 2);
                enemy_player->push_command(enemy_step);
                ++enemy_steps_it;
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
                        *variant = new SimVariance(new CarState(my_player->get_car()),  steps, tick + step);
                    } else if ((*variant)->is_weakness(tick + step, my_player->get_car()))  {
                        delete *variant;
                        *variant = new SimVariance(new CarState(my_player->get_car()),  steps, tick + step);
                    }

                    break;
                } else if (my_player->is_dead()) {
                    break;
                }
            }

            my_player->clear_command_queue();
            enemy_player->clear_command_queue();

            if (!win && deep + step < Constants::MAX_SIMULATION_DEEP && !my_player->is_dead()) {
                run(match, my_player, enemy_player, steps, variant, tick + step, deep + step, enemy_steps);
            } else if (!win && !my_player->is_dead() && deep + step >= Constants::MAX_SIMULATION_DEEP) {//cant go deeper

                if (*variant == NULL) {
                    *variant = new SimVariance(new CarState(my_player->get_car()),  steps, tick + step);
                } else {
                    if ((*variant)->is_weakness(tick + step, my_player->get_car())) {
                        delete *variant;
                        *variant = new SimVariance(new CarState(my_player->get_car()),  steps, tick + step);
                    }
                }
            }

            my_player->set_dead(false);
            enemy_player->set_dead(false);
            steps.pop_back();
            restore_heap();
        }
    }
};



/*
 *
93778
88919
85841
 */

#endif //MADCARS_SIMULATION_H
