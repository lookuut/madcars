//
// Created by lookuut on 21.09.18.
//

#include "FutureSimulation.h"

int FutureSimulation::recursive_run(
        list<short> steps,
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
    list<short> win_stats;

    int max_win_count = -10000000;
    int win_count_sum = 0;

    for (short command = 0; command <= Constants::STEP_MAX_SIZE; command++) {
        int win_count = 0;
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

        for (int i = 0; i < simulation_step_sizes[deep - 1]; i++) {
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
                win_count += pow(3, max_deep - deep);
                break;
            } else if (my_player->is_dead()) {
                win_count -= pow(3, max_deep - deep);;
                break;
            }
        }

        my_player->clear_command_queue();
        enemy_player->clear_command_queue();

        if (!win && deep < simulation_step_sizes.size() && !my_player->is_dead()) {
            win_count += recursive_run(
                    steps,
                    tick + step,
                    deep + 1,
                    enemy_steps,
                    enemy_step_pos,
                    step_states,
                    list<short>(),
                    list<short>());
        }


        if (max_win_count < win_count && deep == 1) {
            max_win_count = win_count;
            this->max_win_count_commands = steps;

            if (this->command_states->size() > 0) {
                command_states->remove_if(deleteAll);
            }

            for (std::list<CarState*>::iterator it = step_states->begin(); it != step_states->end(); ++it) {
                if (*it != NULL) {
                    this->command_states->push_back( new CarState( **it ) );
                }
            }

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


        win_count_sum += win_count;
    }

    return win_count_sum;
}

int FutureSimulation::run() {

    list<short>::iterator enemy_steps_iter = enemy_steps->begin();

    list<CarState*>  * step_states = new list<CarState*>();

    int result = recursive_run(list<short>(), tick, 1, enemy_steps, enemy_steps_iter, step_states, *my_start_commands, *enemy_start_commands);

    step_states->remove_if(deleteAll);
    delete step_states;

    return result;
}

list<short> FutureSimulation::get_steps() {
    return this->max_win_count_commands;
}

list<CarState*> * FutureSimulation::get_states() {
    return this->command_states;
}