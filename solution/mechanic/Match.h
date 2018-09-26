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
    list<Player*> players;
    int map_id;

public:
    list<Player*> dead_players;

    Deadline * deadline;
    int ticks_to_deadline = Constants::TICKS_TO_DEADLINE;
    bool is_rest = false;
    int rest_counter = 0;

    Match();

    Match(const json & world_config, const json & tick_config, list<Player*> players, Player * my_player, Player * enemy_player, cpSpace * space) {
        this->space = space;
        this->my_player = my_player;
        this->enemy_player = enemy_player;
        this->players  = players;

        this->map = new Map(world_config["params"]["proto_map"]["segments"] , space);
        this->map_objects = map->get_objects_for_space();
        this->deadline = new Deadline(ASC, 1200, 800);
        this->map_objects.push_back(this->deadline->get_object_for_space());

        int ally_modification = tick_config["params"]["my_car"][2].get<int>();

        this->map_id = world_config["params"]["proto_map"]["external_id"];

        if (ally_modification == 1) {
            Car * allyCar = new Car(world_config["params"]["proto_car"], 2, ally_modification, space, map);

            cpVect ally_car_start_pos = cpv(tick_config["params"]["my_car"][0][0].get<double>(), tick_config["params"]["my_car"][0][1].get<double>());

            this->my_player->set_car(allyCar);
            this->my_player->clear_command_queue();
            this->my_player->set_dead(false);

            Car * enemyCar = new Car(world_config["params"]["proto_car"], 1, tick_config["params"]["enemy_car"][2].get<int>(), space, map);

            cpVect enemy_car_start_pos = cpv(tick_config["params"]["enemy_car"][0][0].get<double>(), tick_config["params"]["enemy_car"][0][1].get<double>());


            this->enemy_player->set_car(enemyCar);
            this->enemy_player->clear_command_queue();
            this->enemy_player->set_dead(false);


            cpCollisionHandler * a_handler = cpSpaceAddWildcardHandler(space, allyCar->get_button_collision_type());
            a_handler->beginFunc = first_player_callback;
            a_handler->userData = this;

            cpCollisionHandler * e_handler = cpSpaceAddWildcardHandler(space, enemyCar->get_button_collision_type());
            e_handler->beginFunc = second_player_callback;
            e_handler->userData = this;

            car_objects.push_back(allyCar->get_objects_for_space_at(ally_car_start_pos));
            car_objects.push_back(enemyCar->get_objects_for_space_at(enemy_car_start_pos));
        } else {
            Car * enemyCar = new Car(world_config["params"]["proto_car"], 1, tick_config["params"]["enemy_car"][2].get<int>(), space, map);

            cpVect enemy_car_start_pos = cpv(tick_config["params"]["enemy_car"][0][0].get<double>(), tick_config["params"]["enemy_car"][0][1].get<double>());

            this->enemy_player->set_car(enemyCar);
            this->enemy_player->clear_command_queue();
            this->enemy_player->set_dead(false);

            Car * allyCar = new Car(world_config["params"]["proto_car"], 2, ally_modification, space, map);

            cpVect ally_car_start_pos = cpv(tick_config["params"]["my_car"][0][0].get<double>(), tick_config["params"]["my_car"][0][1].get<double>());

            this->my_player->set_car(allyCar);
            this->my_player->clear_command_queue();
            this->my_player->set_dead(false);


            cpCollisionHandler * e_handler = cpSpaceAddWildcardHandler(space, enemyCar->get_button_collision_type());
            e_handler->beginFunc = second_player_callback;
            e_handler->userData = this;

            cpCollisionHandler * a_handler = cpSpaceAddWildcardHandler(space, allyCar->get_button_collision_type());
            a_handler->beginFunc = first_player_callback;
            a_handler->userData = this;

            car_objects.push_back(enemyCar->get_objects_for_space_at(enemy_car_start_pos));
            car_objects.push_back(allyCar->get_objects_for_space_at(ally_car_start_pos));
        }
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

    void clear_dead_players() {
        dead_players = list<Player*>();
    }

    void apply_turn_wrapper(Player * player, int tick) {
        if (!this->is_rest) {
            player->apply_turn(tick);
        }
    }

    void tick(int match_tick) {

        for (std::list<Player*>::iterator it = players.begin(); it != players.end(); ++it) {
            apply_turn_wrapper(*it, match_tick);
        }

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

    int get_map_id() {
        return map_id;
    }

    bool butt_beware() {
        return (get_map_id() == 6 || get_map_id() == 1 || get_map_id() == 5) && (get_my_player()->get_car()->get_external_id() == 2);
    }

    int butt_beware_wait_tick() {
        if (get_map_id() == 6) {
            return 100;
        } else if (get_map_id() == 1 || get_map_id() == 5) {
            return 40;
        }
    }
};


#endif //MADCARS_MATCH_H
