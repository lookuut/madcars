//
// Created by lookuut on 21.09.18.
//

#ifndef MADCARS_FUTURESIMULATION_H
#define MADCARS_FUTURESIMULATION_H



#include <chipmunk/chipmunk_structs.h>
#include "Match.h"
#include "CarState.h"
#include "madcars_allocator.h"
#include "SimVariance.h"
#include <algorithm>    // std::max
#include "BaseSimulation.h"

typedef struct md_heap_state md_heap_state;

class FutureSimulation : public BaseSimulation {

private:
    Match * match;

    Player * my_player;
    Player * enemy_player;

    list<short> * my_start_commands;
    list<short> * enemy_start_commands;
    list<short> * enemy_steps;
    int tick;
    int max_deep;
    list<short> max_win_count_commands;
    list<CarState> max_win_command_states;

    int max_win_count = -100000;

    std::tuple<int, list<short>, list<CarState>> recursive_run(
            list<short> steps,
            int tick,
            int deep,
            list<short> * enemy_steps,
            list<short>::iterator &enemy_step_pos,
            list<CarState> * step_states,
            list<short> my_start_commands,
            list<short> enemy_start_commands
    );

public:

    FutureSimulation(Match * match, list<short> * my_start_commands, list<short> * enemy_start_commands, list<short > * enemy_steps, int tick) {
        this->match = match;
        this->my_player = match->get_my_player();
        this->enemy_player = match->get_enemy_player();
        this->my_start_commands = my_start_commands;
        this->enemy_start_commands = enemy_start_commands;
        this->enemy_steps = enemy_steps;
        this->tick = tick;
        this->max_deep = this->simulation_step_sizes.size();
        this->max_win_command_states = list<CarState>();
    }

    int run ();

    list<short> get_steps();
    list<CarState> get_states();
};


#endif //MADCARS_FUTURESIMULATION_H
