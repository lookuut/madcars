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

    void set_step_sizes(vector<int> simulation_step_sizes) {
        this->simulation_step_sizes = simulation_step_sizes;
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
            list<short>::iterator &enemy_step_pos,
            list<CarState *> * step_states,
            list<short> my_start_commands,
            list<short> enemy_start_commands
            ) {

        bool win = false;
        list<short> prev_command_steps = steps;

        for (short command = 0; command <= Constants::STEP_MAX_SIZE; command++) {
            copy_heap();

            if (my_start_commands.size() > 0) {
                list<short>::iterator enemy_it = enemy_start_commands.begin();

                for (list<short>::iterator it = my_start_commands.begin(); it != my_start_commands.end(); ++it, ++enemy_it) {
                    my_player->push_command(*it);
                    enemy_player->push_command(*enemy_it);
                }
            }

            list<CarState *>::iterator step_states_tail = (step_states->size() > 0 ? --step_states->end() : step_states->begin());

            my_player->set_dead(false);
            enemy_player->set_dead(false);

            for (int i = 0; i < simulation_step_sizes[deep]; i++) {
                my_player->push_command(command);

                short enemy_step = (enemy_step_pos != enemy_steps->end() ? *enemy_step_pos : Constants::default_step);
                enemy_player->push_command(enemy_step);
                ++enemy_step_pos;
            }

            size_t f_size = my_player->command_queue_size();
            size_t s_size = enemy_player->command_queue_size();

            int step = 0;
            for(int command_number = 0; command_number < std::max(s_size, f_size); command_number++, step++) {

                steps.push_back(my_player->get_first_command());
                match->tick(tick + step);

                cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

                step_states->push_back(new CarState(my_player->car));

                if (enemy_player->is_dead() && !my_player->is_dead()) {/// find situation where we win
                    win = true;

                    if (*variant == NULL) {
                        *variant = new SimVariance(new CarState(my_player->get_car()), new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes, step_states);
                    } else if ((*variant)->is_weakness(tick + step, my_player->get_car(), enemy_player->get_car()))  {
                        delete *variant;
                        *variant = new SimVariance(new CarState(my_player->get_car()), new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes, step_states);
                    }

                    break;
                } else if (my_player->is_dead()) {
                    break;
                }
            }

            my_player->clear_command_queue();
            enemy_player->clear_command_queue();

            if (!win && deep + 1 < simulation_step_sizes.size() && !my_player->is_dead()) {
                run(match, my_player, enemy_player, steps, variant, tick + step, deep + 1, enemy_steps, enemy_step_pos, step_states, list<short>(), list<short>());
            } else if (!win && !my_player->is_dead() && deep + 1 >= simulation_step_sizes.size()) {//cant go deeper

                if (*variant == NULL) {
                    *variant = new SimVariance(new CarState(my_player->get_car()), new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes, step_states);
                } else {
                    if ((*variant)->is_weakness(tick + step, my_player->get_car(), enemy_player->get_car()) ) {
                        delete *variant;
                        *variant = new SimVariance(new CarState(my_player->get_car()),  new CarState(enemy_player->get_car()), steps, tick + step, &simulation_step_sizes, step_states);
                    }
                }
            } else if (*variant == NULL) { // trash for no variant
                *variant = new SimVariance(new CarState(my_player->get_car()), new CarState(enemy_player->get_car()), steps, tick + 10000, &simulation_step_sizes, step_states);
            }

            for (std::list<CarState*>::iterator it = --step_states->end(); it != step_states_tail; --it) {
                delete *it;
                step_states->erase(it);
            }

            my_player->set_dead(false);
            enemy_player->set_dead(false);
            match->clear_dead_players();
            steps = prev_command_steps;
            restore_heap();
        }
    }


    int check_future_steps (list<short> * steps, list<short> * enemy_steps, list<CarState*> *states, Match * match, int tick) {

        copy_heap();

        list<short>::iterator enemy_step_iter = enemy_steps->begin();
        list<short>::iterator step_iter = steps->begin();
        list<CarState*>::iterator state_iter = states->begin();

        bool is_sima_correct = true;
        int not_correct_tick = 0;

        while (step_iter != steps->end() && state_iter != states->end()) {

            short enemy_step = Constants::CAR_STOP_COMMAND;

            if (enemy_step_iter != enemy_steps->end()) {
                enemy_step = *enemy_step_iter;
            }

            match->get_my_player()->push_command(*step_iter);
            match->get_enemy_player()->push_command(enemy_step);
            match->tick(tick + step);

            cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

            if (!(*state_iter)->is_equal(match->get_my_player()->get_car())) {
                break;
            }

            not_correct_tick++;
            ++state_iter;
            ++step_iter;
        }

        restore_heap();

        return not_correct_tick;
    }
};


#endif //MADCARS_SIMULATION_H
