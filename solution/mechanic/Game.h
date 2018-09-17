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


using namespace nlohmann;
using std::chrono::system_clock;

class Game {
private:
    tuple<list<cpShape *>,list<car_objects_type*>> match_objects;

    cpSpace* space = NULL;

    short max_match_count = 3;

    int tick_num = 0;
    Simulation simulation;

    list<CarState *> enemy_states;

    list<short> my_future_steps;
    list<short> my_past_steps;

    int match_tick = 0;

    Player * my_player = NULL;
    Player * enemy_player = NULL;

    Match * current_match = NULL;
public:

    static const short default_step = 2;
    static const int WAIT_STEP_SIZE = 10;

    ~Game() {
        delete this->current_match;
    }

    Game()  {
        this->space = cpSpaceNew();

        cpSpaceSetGravity(this->space, cpv(0.0, -700.0));
        cpSpaceSetDamping(this->space, 0.85);
    }

    static bool deleteAll( CarState * theElement ) { delete theElement; return true; }

    void tick(json & _config) {
        try {

            int micros = 0;
            system_clock::time_point start = system_clock::now();

            CarState my_car_state(_config["params"]["my_car"]);
            CarState * enemy_car_state = new CarState(_config["params"]["enemy_car"]);

            this->enemy_states.push_back(enemy_car_state);

            if (enemy_states.size() >= WAIT_STEP_SIZE) {// need synchronized
                list<short>::iterator my_steps_iter = this->my_past_steps.begin();
                list<CarState*>::iterator enemy_steps_iter = this->enemy_states.begin();

                Player * f_player = this->current_match->get_my_player();
                Player * s_player = this->current_match->get_enemy_player();

                f_player->clear_command_queue();
                s_player->clear_command_queue();
                //state of world on prev tick
                list<short> * enemy_steps = simulation.synchronizedStates(&this->my_past_steps, &this->enemy_states,
                                                                          my_steps_iter, enemy_steps_iter,
                                                                          this->current_match, 0);
                if (enemy_steps == NULL) {
                    throw std::invalid_argument("Cant synchronized enemy steps");
                }

                //clear applyed steps and enemy states except last
                short last_step = my_past_steps.back();
                my_past_steps = list<short>();
                my_past_steps.push_back(last_step);

                CarState * state = enemy_states.back();
                enemy_states.pop_back();
                enemy_states . remove_if ( deleteAll );

                enemy_states = list<CarState*>();
                enemy_states.push_back(state);

                SimVariance * variant = NULL;


                f_player->push_command(last_step);
                s_player->push_command(Game::default_step);

                simulation.run(this->current_match, f_player, s_player, list<short>(), &variant, this->match_tick, 0, enemy_steps);
                delete enemy_steps;

                if (variant == NULL) {//cannot find any variants
                    my_future_steps.push_back(10);
                    my_future_steps.push_back(10);
                    my_future_steps.push_back(10);
                    my_future_steps.push_back(10);
                } else {
                    my_future_steps = variant->get_steps();
                }
            }

            short step = get_future_step();
            add_past_step(step);

            micros = std::chrono::duration_cast<std::chrono::microseconds>(system_clock::now() - start).count();

            Game::send_command(step, "Tick " + to_string(this->tick_num) + " step " + Player::commands[step]  + " calculated in " + std::to_string(micros));

            this->tick_num++;
            this->match_tick++;

        } catch (const std::exception& e) {
            Game::send_command(1, e.what());
        }
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

        if (my_past_steps.size() > 0) {

            list<short>::iterator my_steps_iter = this->my_past_steps.begin();
            list<CarState*>::iterator enemy_steps_iter = this->enemy_states.begin();


            this->my_player->clear_command_queue();
            this->enemy_player->clear_command_queue();
            //state of world on prev tick
            list<short> * enemy_steps = simulation.synchronizedStates(&this->my_past_steps, &this->enemy_states,
                                                                      my_steps_iter, enemy_steps_iter,
                                                                      this->current_match, 0);

            this->my_player->push_command(my_past_steps.back());
            this->enemy_player->push_command(2);

            this->current_match->tick(this->match_tick);
            this->match_tick++;

            cpSpaceStep(space, Constants::SPACE_TICK);
            if (!this->current_match->is_rest && this->current_match->smbd_die()) {
                this->current_match->rest_counter = Constants::REST_TICKS;
                this->current_match->is_rest = true;

                while (this->current_match->rest_counter > 0) {
                    this->current_match->rest_counter--;

                    this->current_match->deadline_move_tick(this->match_tick);
                    cpSpaceStep(this->space, Constants::SPACE_TICK);
                    this->match_tick++;
                }
            }
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

        my_player = new Player(1, max_match_count, true, true);
        enemy_player = new Player(2, max_match_count, false, true);


        if (enemy_states.size()) {
            enemy_states.remove_if(deleteAll);
        }

        enemy_states = list<CarState*>();

        my_future_steps = list<short>();
        my_past_steps = list<short>();
        match_tick = 0;


        current_match = new Match(world_config, tick_config, this->my_player, this->enemy_player, this->space, true);
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

    short get_past_step() {
        short step = my_future_steps.front();
        my_past_steps.pop_front();
        return step;
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

    static void send_command(const short command, const string & debug) {

        nlohmann::json json_command;

        json_command["command"] = Player::commands[command];
        json_command["debug"] = debug;

        cout << json_command.dump() << endl;
    }
};


#endif //MADCARS_GAME_H
