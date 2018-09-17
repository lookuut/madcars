//
// Created by lookuut on 09.09.18.
//

#ifndef MADCARS_MATCH_H
#define MADCARS_MATCH_H

#include <list>
#include <chipmunk/chipmunk_structs.h>
#include "../../../nlohmann/json.hpp"
#include "Player.h"
#include "../models/Constans.h"
#include "../models/Map.h"
#include "../models/Deadline.h"

using namespace nlohmann;
using namespace std;

class Match {

private:
    cpSpace* space;

    Player * my_player;
    Player * enemy_player;

    list<cpShape*> map_objects;
    list<car_objects_type*> car_objects;
    Map * map;
public:
    list<Player*> dead_players;

    Deadline * deadline;
    int ticks_to_deadline = Constants::TICKS_TO_DEADLINE;
    bool is_rest = false;
    int rest_counter = 0;

    Match();

    Match(const json & world_config, const json & tick_config, Player * my_player, Player * enemy_player, cpSpace * space, bool apply_handler) {
        this->space = space;
        this->my_player = my_player;
        this->enemy_player = enemy_player;

        this->map = new Map(world_config["params"]["proto_map"]["segments"] , space);
        this->map_objects = map->get_objects_for_space();
        this->deadline = new Deadline(ASC, 1800, 800);
        this->map_objects.push_back(this->deadline->get_object_for_space());

        Car * allyCar = new Car(world_config["params"]["proto_car"], 1, tick_config["params"]["my_car"][2].get<int>(), space);

        cpVect car_start_pos = cpv(tick_config["params"]["my_car"][0][0].get<double>(), tick_config["params"]["my_car"][0][1].get<double>());
        car_objects.push_back(allyCar->get_objects_for_space_at(car_start_pos));

        if (apply_handler) {
            cpCollisionHandler * handler = cpSpaceAddWildcardHandler(space, allyCar->get_button_collision_type());
            handler->beginFunc = first_player_callback;
            handler->userData = this;
        }

        this->my_player->set_car(allyCar);
        this->my_player->clear_command_queue();
        this->my_player->set_dead(false);

        Car * enemyCar = new Car(world_config["params"]["proto_car"], 2, tick_config["params"]["enemy_car"][2].get<int>(), space);

        car_start_pos = cpv(tick_config["params"]["enemy_car"][0][0].get<double>(), tick_config["params"]["enemy_car"][0][1].get<double>());
        car_objects.push_back(enemyCar->get_objects_for_space_at(car_start_pos));

        if (apply_handler) {
            cpCollisionHandler * handler = cpSpaceAddWildcardHandler(space, enemyCar->get_button_collision_type());
            handler->beginFunc = second_player_callback;
            handler->userData = this;
        }

        this->enemy_player->set_car(enemyCar);
        this->enemy_player->clear_command_queue();
        this->enemy_player->set_dead(false);

    }


    ~Match() {
        delete this->map;
        delete this->deadline;
    }

    static cpBool first_player_callback(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
        Match * match = (Match*)userData;

        if (!match->is_rest) {
            match->dead_players.push_back(match->get_my_player());
            match->get_my_player()->set_dead(true);
        }

        return false;
    }

    static cpBool second_player_callback(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {

        Match * match = (Match*)userData;
        if (!match->is_rest) {
            match->dead_players.push_back(match->get_enemy_player());
            match->get_enemy_player()->set_dead(true);
        }

        return false;
    }

    tuple<list<cpShape *>, list<car_objects_type*>> get_object_for_space() {
        return make_tuple(this->map_objects, this->car_objects);
    }

    void apply_turn_wrapper(Player * player, int tick) {
        if (!this->is_rest) {
            player->apply_turn(tick);
        }
    }

    void tick(int match_tick) {

        apply_turn_wrapper(this->my_player, match_tick);
        apply_turn_wrapper(this->enemy_player, match_tick);

        if (match_tick >= Constants::TICKS_TO_DEADLINE) {
            this->deadline->move();
        }
    }

    void deadline_move_tick(int match_tick) {
        if (match_tick >= Constants::TICKS_TO_DEADLINE) {
            this->deadline->move();
        }
    }

    bool smbd_die() {
        return this->dead_players.size() > 0;
    }

    bool is_match_ended() {
        return this->rest_counter == 0 && this->is_rest && this->smbd_die();
    }

    cpSpace * get_space() {
        return this->space;
    }

    Player * get_my_player() {
        return this->my_player;
    }

    Player * get_enemy_player() {
        return this->enemy_player;
    }

};


#endif //MADCARS_MATCH_H
