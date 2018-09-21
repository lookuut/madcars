//
// Created by lookuut on 13.09.18.
//

#include "BaseSimulation.h"


list<short> BaseSimulation::win_definer(short my_command, short winner, Match *match, int tick) {

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

short BaseSimulation::enemy_step_definer(short my_prev_command, CarState *my_state, CarState *enemy_state, Match *match, int tick, int round) {

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


int BaseSimulation::check_future_steps (list<short> * steps, list<short> * enemy_steps, list<CarState*> *states, Match * match, int tick) {

    copy_heap();

    list<short>::iterator enemy_step_iter = enemy_steps->begin();
    list<short>::iterator step_iter = steps->begin();
    list<CarState*>::iterator state_iter = states->begin();

    int not_correct_tick = 0;
    int step = 0;
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
        step++;
    }

    restore_heap();

    return not_correct_tick;
}