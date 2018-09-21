//
// Created by lookuut on 09.09.18.
//

#ifndef MADCARS_GAME_H
#define MADCARS_GAME_H

#include <iostream>

#include <list>
#include <vector>

#include "../../../nlohmann/json.hpp"
#include "Player.h"
#include "Match.h"
#include "Simulation.h"

#include <chipmunk/chipmunk_structs.h>
#include <chrono>

#include "../utils/easylogging++.h"

using namespace nlohmann;
using std::chrono::system_clock;

class Game {
private:
    tuple<list<cpShape *>,list<car_objects_type*>> match_objects;

    cpSpace* space = NULL;

    int round = 0;

    Simulation simulation;

    list<short> my_future_steps;
    list<CarState*> * my_future_states = NULL;

    int match_tick = 0;
    int micro_sum = 0;

    list<Player*> players;

    Player * my_player = NULL;
    Player * enemy_player = NULL;

    Match * current_match = NULL;

    list<short> enemy_steps;
    list<short> future_enemy_steps;

    short my_lives;
    short enemy_lives;

public:


    ~Game() {
        delete this->current_match;
    }

    Game() {
        this->space = cpSpaceNew();

        cpSpaceSetGravity(this->space, cpv(0.0, -700.0));
        cpSpaceSetDamping(this->space, 0.85);

        my_future_states = new list<CarState*>();
    }

    static bool deleteAll( CarState * theElement ) { delete theElement; return true; }

private:
    string message = "";
public:

    vector<int> default_step_size{40, 40 , 40 , 40, 40};
    vector<int> in_danger_steps_sizes{15, 15, 15, 15};

    int last_forecast_size = 0;

    void forecast(int not_correct_tick) {

        SimVariance * variant = NULL;

        list<short>::iterator enemy_steps_iter = enemy_steps.begin();

        list<CarState*>  * step_states = new list<CarState*>();

        list<short> my_start_steps;
        list<short> enemy_start_steps;

        if (my_future_steps.size() > 0) {
            my_player->push_command(get_first_step());
            enemy_player->push_command(*enemy_steps_iter);

            my_start_steps.push_back(get_first_step());
            enemy_start_steps.push_back(*enemy_steps_iter);

            ++enemy_steps_iter;
        }

        simulation.set_step_sizes( (not_correct_tick <= 0 ? default_step_size : in_danger_steps_sizes));
        simulation.run(this->current_match, my_player, enemy_player, list<short>(), &variant, this->match_tick, 0, &enemy_steps, enemy_steps_iter, step_states, my_start_steps, enemy_start_steps);

        #ifdef LOCAL_RUNNER
        LOG(INFO) << "Simulation  " << variant->tick;
        #endif

        step_states->remove_if(deleteAll);
        delete step_states;

        if (my_future_states->size() > 0) {
            my_future_states->remove_if(deleteAll);
            delete my_future_states;
        }

        my_future_steps = variant->get_steps();
        my_future_states = variant->get_states();
        last_forecast_size = my_future_states->size();

        future_enemy_steps = enemy_steps;

    }

    void tick(json & _config) {

        message = "";

        int micros = 0;
        system_clock::time_point start = system_clock::now();

        CarState * enemy_car_state = new CarState(_config["params"]["enemy_car"]);
        CarState my_car_state(_config["params"]["my_car"]);

        if (match_tick >= 1) {

            short prev_enemy_step = simulation.enemy_step_definer(get_first_step(), &my_car_state, enemy_car_state, current_match, this->match_tick, this->round);
            first_step_applied();

            enemy_steps.push_back(prev_enemy_step);

            if (enemy_steps.size() >= 20) {
                enemy_steps.pop_front();
            }

            int not_correct_tick = simulation.check_future_steps(&my_future_steps, &future_enemy_steps, my_future_states, current_match, match_tick);

            if (not_correct_tick < my_future_states->size() - 1) {
                forecast(not_correct_tick);
            } else if ((last_forecast_size > 100 && my_future_steps.size() < last_forecast_size / 2) || my_future_steps.size()  <= 1) {
                forecast(-1);
            }
        }

        short step = get_future_step_to_send();

        micros = std::chrono::duration_cast<std::chrono::microseconds>(system_clock::now() - start).count();
        micro_sum += micros;

        send_command(step, "Round = " + to_string(round) + " Tick " + to_string(this->match_tick) + " "+ std::to_string(micro_sum/1000000.0) + " step " + Player::commands[step]  + " calculated in " + std::to_string(micros) + " : " + message);

        this->match_tick++;
    }



    void clear_spaces() {
        this->clear_space(this->space);
    }

    void clear_space(cpSpace* space){

        list<cpShape *> shapes_list = get<list<cpShape *>>(this->match_objects);

        for (std::list<cpShape*>::iterator it = shapes_list.begin(); it != shapes_list.end(); ++it) {
            cpSpaceRemoveShape(space, *it);
        }

        list<car_objects_type*> car_objects = get< list<car_objects_type*> >(match_objects);
        for (std::list<car_objects_type*>::iterator it = car_objects.begin(); it != car_objects.end(); ++it) {

            cpSpaceRemoveShape(space, (*it)->button_shape);
            cpSpaceRemoveBody(space, (*it)->car_body);

            cpSpaceRemoveShape(space, (*it)->car_shape);
            cpSpaceRemoveBody(space, (*it)->rear_wheel_body);
            cpSpaceRemoveBody(space, (*it)->front_wheel_body);

            cpSpaceRemoveShape(space, (*it)->rear_wheel->wheel_shape);
            cpSpaceRemoveConstraint(space, (*it)->rear_wheel->wheel_groove);
            cpSpaceRemoveConstraint(space, (*it)->rear_wheel->wheel_damp);

            cpSpaceRemoveShape(space, (*it)->front_wheel->wheel_shape);
            cpSpaceRemoveConstraint(space, (*it)->front_wheel->wheel_groove);
            cpSpaceRemoveConstraint(space, (*it)->front_wheel->wheel_damp);

            for (std::list<cpConstraint *>::iterator motor = (*it)->motors.begin(); motor != (*it)->motors.end(); ++motor) {
                cpSpaceRemoveConstraint(space, (*motor));
            }
        }
    }

    void next_match(const json & world_config, const json & tick_config) {

        if (round > 0) {

            short cur_my_lives = my_lives = world_config["params"]["my_lives"].get<int>();
            short cur_enemy_lives = world_config["params"]["enemy_lives"].get<int>();

            short winner = (my_lives == cur_my_lives) ? 1 : ((enemy_lives == cur_enemy_lives) ? 2 : 0);

            list<short> win_steps = simulation.win_definer(get_first_step(), winner, this->current_match, this->match_tick);
            first_step_applied();

            this->my_player->push_command(get_future_step_to_send());
            this->enemy_player->push_command(0);//boolshit ....
            this->current_match->tick(this->match_tick++);
            cpSpaceStep(this->space, Constants::SPACE_TICK);

            this->current_match->rest_counter = Constants::REST_TICKS ;
            this->current_match->is_rest = true;

            do {
                this->current_match->rest_counter--;
                this->current_match->deadline_move_tick(this->match_tick);
                cpSpaceStep(this->space, Constants::SPACE_TICK);

                this->match_tick++;
            } while (this->current_match->rest_counter > 0);

            if (my_future_steps.size() > 0) {
                my_future_states->remove_if(deleteAll);
                delete my_future_states;

                my_future_steps = list<short>();
                my_future_states = new list<CarState*>();

            }

            future_enemy_steps = list<short>();
            enemy_steps = list<short>();
        }

        clear_spaces();

        if (my_player != NULL) {
            delete my_player;
        }

        if (enemy_player != NULL) {
            delete enemy_player;
        }

        if (current_match != NULL) {
            delete current_match;
        }

        players = list<Player*>();

        my_player = new Player(1);
        enemy_player = new Player(2);

        if (tick_config["params"]["my_car"][2] == 1) {
            players.push_back(my_player);
            players.push_back(enemy_player);
        } else {
            players.push_back(enemy_player);
            players.push_back(my_player);
        }

        my_future_steps = list<short>();

        match_tick = 0;
        round++;

        my_lives = world_config["params"]["my_lives"].get<int>();
        enemy_lives = world_config["params"]["enemy_lives"].get<int>();

        current_match = new Match(world_config, tick_config, players, this->my_player, this->enemy_player, this->space);
        match_objects  = current_match->get_object_for_space();

        list<cpShape *> shapes_list = get<list<cpShape *>>(this->match_objects);

        for (std::list<cpShape*>::iterator it = shapes_list.begin(); it != shapes_list.end(); ++it) {
            cpSpaceAddShape(this->space, *it);
        }

        list<car_objects_type*> car_objects = get< list<car_objects_type*> >(match_objects);
        for (std::list<car_objects_type*>::iterator it = car_objects.begin(); it != car_objects.end(); ++it) {

            addShapeToSpace(this->space, (*it)->button_shape);
            addBodyToSpace(this->space, (*it)->car_body);

            addShapeToSpace(this->space, (*it)->car_shape);
            addBodyToSpace(this->space, (*it)->rear_wheel_body);
            addBodyToSpace(this->space, (*it)->front_wheel_body);

            addShapeToSpace(this->space, (*it)->rear_wheel->wheel_shape);
            addConstraitToSpace(this->space, (*it)->rear_wheel->wheel_groove);
            addConstraitToSpace(this->space, (*it)->rear_wheel->wheel_damp);

            addShapeToSpace(this->space, (*it)->front_wheel->wheel_shape);
            addConstraitToSpace(this->space, (*it)->front_wheel->wheel_groove);
            addConstraitToSpace(this->space, (*it)->front_wheel->wheel_damp);

            for (std::list<cpConstraint *>::iterator motor = (*it)->motors.begin(); motor != (*it)->motors.end(); ++motor) {
                addConstraitToSpace(this->space, (*motor));
            }
        }
    }


    void add_future_step(short step) {
        my_future_steps.push_back(step);
    }

    short get_first_step() {
        return my_future_steps.front();
    }

    void first_step_applied() {
        my_future_steps.pop_front();

        my_future_states->pop_front();

    }

    short get_future_step_to_send() {
        return *(my_future_steps.begin().operator++());
    }

    static void addBodyToSpace(cpSpace * space, cpBody * body) {
        cpSpaceAddBody(space, body);
    }

    static void addShapeToSpace(cpSpace * space, cpShape * shape) {
        cpSpaceAddShape(space, shape);
    }

    static void addConstraitToSpace(cpSpace * space, cpConstraint * constraint) {
        cpSpaceAddConstraint(space, constraint);
    }

    void send_command(const short command, const string & debug) {
        nlohmann::json json_command;

        json_command["command"] = Player::commands[command];
        json_command["debug"] = debug;

#ifdef LOCAL_RUNNER
        LOG(INFO) << debug << endl;
#endif
        cout << json_command.dump() << endl;
    }

    void run_empty(short command) {
        my_player->press_command(command);
        enemy_player->press_command(Constants::CAR_STOP_COMMAND);

        current_match->tick(this->match_tick);

        cpSpaceStep(current_match->get_space(), Constants::SPACE_TICK);
        this->match_tick++;
    }
};


#endif //MADCARS_GAME_H
