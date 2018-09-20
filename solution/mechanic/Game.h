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
    list<short> my_past_steps;

    int match_tick = 0;
    int micro_sum = 0;

    list<Player*> players;

    Player * my_player = NULL;
    Player * enemy_player = NULL;

    Match * current_match = NULL;

    list<short> enemy_steps;

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
    }

    static bool deleteAll( CarState * theElement ) { delete theElement; return true; }

    void tick(json & _config) {

        string message = "";
        int micros = 0;
        system_clock::time_point start = system_clock::now();

        CarState * enemy_car_state = new CarState(_config["params"]["enemy_car"]);
        CarState my_car_state(_config["params"]["my_car"]);


        if (my_past_steps.size() >= 2) {

            short prev_enemy_step = simulation.enemy_step_definer(get_prev_step(), &my_car_state, enemy_car_state, current_match, this->match_tick, this->round);
            enemy_steps.push_back(prev_enemy_step);

            if (enemy_steps.size() >= 20) {
                enemy_steps.pop_front();
            }
        }

        if (enemy_steps.size() > 10 && (this->match_tick % 10 == 0 || my_future_steps.size() == 0)) {
            SimVariance * variant = NULL;

            list<short>::iterator enemy_steps_iter = enemy_steps.begin();

            simulation.run(this->current_match, my_player, enemy_player, list<short>(), &variant, this->match_tick, 0, &enemy_steps, enemy_steps_iter);

            if (variant == NULL) {//cannot find any variants
                my_future_steps.push_back(1);
                my_future_steps.push_back(0);
                my_future_steps.push_back(1);
                my_future_steps.push_back(0);
                my_future_steps.push_back(1);
                message += " cant find any variants";
            } else {
                my_future_steps = variant->get_steps();
            }
        }

        short step = get_future_step();

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

            list<short> win_steps = simulation.win_definer(this->get_prev_step(2), winner, this->current_match, this->match_tick);

            this->my_player->push_command(this->get_prev_step(2));
            this->enemy_player->push_command(0);//boolshit ....
            this->current_match->tick(this->match_tick++);
            cpSpaceStep(this->space, Constants::SPACE_TICK);

            this->my_player->push_command(my_past_steps.back());
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
        my_past_steps = list<short>();

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

    short get_future_step() {
        short step = my_future_steps.front();
        my_future_steps.pop_front();
        return step;
    }


    void add_past_step(short step) {
        my_past_steps.push_back(step);
    }

    short get_prev_step(int pos_from_tail) {
        return *(std::prev(my_past_steps.end(), pos_from_tail));
    }

    short get_prev_step() {
        return *((my_past_steps.end().operator--()).operator--());
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

        add_past_step(command);

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
