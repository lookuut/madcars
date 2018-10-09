//
// Created by lookuut on 21.09.18.
//

#include "FutureSimulation.h"

void FutureSimulation::recursive_run(
        list<short> steps,
        int tick,
        int deep,
        list<short> * enemy_steps,
        list<short>::iterator &enemy_step_pos,
        list<short> my_start_commands,
        list<short> enemy_start_commands
) {


    list<short> prev_command_steps = steps;
    list<short> win_stats;

    for (short command = 0; command <= Constants::STEP_MAX_SIZE; command++) {
        char win = 0;

        copy_heap();

        if (my_start_commands.size() > 0) {
            list<short>::iterator enemy_it = enemy_start_commands.begin();

            for (list<short>::iterator it = my_start_commands.begin(); it != my_start_commands.end(); ++it, ++enemy_it) {
                my_player->push_command(*it);
                enemy_player->push_command(*enemy_it);
            }
        }

        my_player->set_dead(false);
        enemy_player->set_dead(false);

        for (int i = 0; i < simulation_step_sizes[deep - 1]; i++) {
            my_player->push_command(command);

            short enemy_step = (enemy_step_pos != enemy_steps->end() ? *enemy_step_pos : Constants::default_step);
            enemy_player->push_command(enemy_step);
            ++enemy_step_pos;
        }

        size_t f_size = my_player->command_queue_size();
        size_t s_size = enemy_player->command_queue_size();

        CarState cur_car_state(my_player->get_car());
        bool nothing_change = false;
        int step = 0;
        int change_car_state_tick = 0;
        for(int command_number = 0; command_number < std::max(s_size, f_size); command_number++, step++) {

            steps.push_back(my_player->get_first_command());

            match->tick(tick + step);

            cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

            if (!cur_car_state.is_change(my_player->get_car()) && step - change_car_state_tick > 5) {
                nothing_change = true;
                break;
            }

            if (cur_car_state.is_change(my_player->get_car())) {
                cur_car_state = CarState(my_player->get_car());
                change_car_state_tick = step;
            }

            if (enemy_player->is_dead() && !my_player->is_dead()) {/// find situation where we win
                win = 1;
                break;
            } else if (my_player->is_dead()) {
                win = -1;
                break;
            }
        }

        CarState car_state = CarState(my_player->car);
        eva.evaluate(tick + step, win, car_state, steps);
        my_player->clear_command_queue();
        enemy_player->clear_command_queue();

        if (!nothing_change && win == 0 && deep < simulation_step_sizes.size() && !my_player->is_dead()) {
            recursive_run(
                    steps,
                    tick + step,
                    deep + 1,
                    enemy_steps,
                    enemy_step_pos,
                    list<short>(),
                    list<short>());
        }

        my_player->set_dead(false);
        enemy_player->set_dead(false);
        match->clear_dead_players();
        steps = prev_command_steps;
        restore_heap();
    }
}

void FutureSimulation::run() {
    if (match->butt_beware()) {

        list<short> steps;
        copy_heap();
        int step = 0;
        if (my_start_commands->size() > 0) {

            list<short>::iterator enemy_it = enemy_start_commands->begin();

            for (list<short>::iterator it = my_start_commands->begin(); it != my_start_commands->end(); ++it, ++enemy_it, step++) {
                my_player->press_command(*it);
                enemy_player->press_command(*enemy_it);

                match->tick(tick + step);
                cpSpaceStep(match->get_space(), Constants::SPACE_TICK);
                steps.push_back(*it);
            }
        }
        int sizes[8] = {3,3,3,3,3,3,3,3};

        int g_step = 0;
        for (int step = 0; step <= 7; step++) {
            double max_height = -10000;
            short max_command;
            for (short command = 0; command <= Constants::CAR_STOP_COMMAND; command++) {
                copy_heap();

                for (int i = 0; i <= sizes[step]; i++) {
                    my_player->press_command(command);
                    enemy_player->press_command(Constants::CAR_STOP_COMMAND);

                    match->tick(tick + g_step + i);

                    cpSpaceStep(match->get_space(), Constants::SPACE_TICK);
                }

                cpSpaceStep(match->get_space(), Constants::SPACE_TICK);

                vector<cpVect> button_shape = my_player->get_car()->get_button_world_coors();
                auto min_point = std::min_element( button_shape.begin(), button_shape.end(),
                                                   []( const cpVect &a, const cpVect &b )
                                                   {
                                                       return a.y < b.y;
                                                   } );


                if(min_point->y > max_height) {
                    max_height = min_point->y;
                    max_command = command;
                }

                restore_heap();
            }

            for (int i = 0; i <= sizes[step]; i++, g_step++) {
                my_player->push_command(max_command);
                enemy_player->push_command(Constants::CAR_STOP_COMMAND);

                match->tick(tick + step);

                cpSpaceStep(match->get_space(), Constants::SPACE_TICK);


                steps.push_back(max_command);
            }
        }

        my_player->set_dead(false);
        enemy_player->set_dead(false);

        restore_heap();

        eva.set_commands(steps);
    } else {
        list<short>::iterator enemy_steps_iter = enemy_steps->begin();

        recursive_run(list<short>(), tick, 1, enemy_steps, enemy_steps_iter,  *my_start_commands, *enemy_start_commands);
    }

}

list<short> FutureSimulation::get_steps() {
    return eva.get_commands();
}
