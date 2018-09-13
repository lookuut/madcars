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
    list<Player*> players;
    list<cpShape*> map_objects;
    list<car_objects_type*> car_objects;

    list<Player> dead_players;


public:

    Deadline * deadline;
    int ticks_to_deadline = Constants::TICKS_TO_DEADLINE;
    bool is_rest = false;
    int rest_counter = 0;

    Match();

    Match(json & config, list<Player*> & players, cpSpace * space, bool apply_handler) {
        this->space = space;
        this->players = players;

        Map * map = new Map(config["params"]["proto_map"]["segments"] , space);
        this->map_objects = map->get_objects_for_space();
        this->deadline = new Deadline(ASC, 1800, 800);
        this->map_objects.push_back(this->deadline->get_object_for_space());

        Car * allyCar = new Car(config["params"]["proto_car"], 1, 0, space);
        car_objects.push_back(allyCar->get_objects_for_space_at(cpv(300, 300)));

        if (apply_handler) {
            cpCollisionHandler * handler = cpSpaceAddWildcardHandler(space, allyCar->get_button_collision_type());
            //handler->beginFunc =
        }

        this->players.front()->set_car(allyCar);

        Car * enemyCar = new Car(config["params"]["proto_car"], 2, 1, space);
        car_objects.push_back(enemyCar->get_objects_for_space_at(cpv(900, 300)));

        if (apply_handler) {
            cpCollisionHandler * handler = cpSpaceAddWildcardHandler(space, enemyCar->get_button_collision_type());
            //handler->beginFunc =
        }

        this->players.back()->set_car(enemyCar);
    }

    ~Match() {
        delete this->deadline;
    }

    tuple<list<cpShape *>,list<car_objects_type*>> get_object_for_space() {
        return make_tuple(this->map_objects, this->car_objects);
    }

    void apply_turn_wrapper(Player * player, int tick) {
        if (!this->is_rest) {
            player->apply_turn(tick);
        }
    }

    void tick(int game_tick) {

        for (std::list<Player*>::iterator it = this->players.begin(); it !=  this->players.end(); ++it) {
            if ((*it)->is_ally) {
                apply_turn_wrapper(*it, game_tick);
            }
        }

        if (this->ticks_to_deadline < 1) {
            this->deadline->move();
        } else {
            this->ticks_to_deadline--;
        }
    }

    bool smbd_die() {
        return this->dead_players.size() > 0;
    }

    bool is_match_ended() {
        this->rest_counter == 0 && this->is_rest && this->smbd_die();
    }

    cpSpace * get_space() {
        return this->space;
    }
};


#endif //MADCARS_MATCH_H
