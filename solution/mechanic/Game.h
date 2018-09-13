//
// Created by lookuut on 09.09.18.
//

#ifndef MADCARS_GAME_H
#define MADCARS_GAME_H

#include <list>
#include <vector>
#include <chipmunk/chipmunk_structs.h>
#include "../../../nlohmann/json.hpp"
#include "Player.h"
#include "Match.h"

#include "../utils/easylogging++.h"

using namespace nlohmann;

class Game {
private:
    tuple<list<cpShape *>,list<car_objects_type*>> match_objects;

    cpSpace* space = NULL;
    json config;
    bool game_complete = false;
    short max_match_count = 3;
    std::list<Player*> players;
    Match * current_match;
    int tick_num = 0;

public:

    ~Game() {
        delete this->current_match;
    }

    Game(json & config)  {
        this->config = config;
        this->space = cpSpaceNew();

        cpSpaceSetGravity(this->space, cpv(0.0, -700.0));
        cpSpaceSetDamping(this->space, 0.85);

        this->players.push_back(new Player(1, max_match_count, true, true));
        this->players.push_back(new Player(2, max_match_count, false, true));

        this->next_match();
    }

    static void EachBody(cpBody *body, cpSpace *space){
        cpSpaceRemoveBody(space, body);
    }

    static void EachShape(cpShape *shape, cpSpace *space){
        cpSpaceRemoveShape(space, shape);
    }

    static void EachConstraint(cpShape *shape, cpSpace *space){
        cpSpaceRemoveShape(space, shape);
    }

    void tick() {
        string input_string = "";
        getline(cin, input_string);

        auto _config = nlohmann::json::parse(input_string);

        if (_config["type"].get<std::string>() == "new_match") {
            this->config = _config;
        }

        if (_config["type"].get<std::string>() == "tick") {
            cpVect lrVec = cpv(_config["params"]["my_car"][0][0].get<double>(), _config["params"]["my_car"][0][1].get<double>());
            cpVect sVec = cpBodyGetPosition(this->players.front()->get_car()->car_body);
            LOG(INFO) << std::setprecision(17) << this->tick_num << " x delta " << (lrVec.x - sVec.x) << " y delta " << (lrVec.y - sVec.y) << " lx:" << lrVec.x << " ly:" << lrVec.y << " sx:" << sVec.x << "sy:" << sVec.y;
        }

        this->current_match->tick(this->tick_num);
        cpSpaceStep(this->space, 0.016);

        if (!this->current_match->is_rest && this->current_match->smbd_die()) {
            this->current_match->rest_counter = Constants::REST_TICKS;
            this->current_match->is_rest = true;

            while (this->current_match->rest_counter > 0) {
                this->current_match->rest_counter--;
                if (this->current_match->ticks_to_deadline < 0) {
                    this->current_match->deadline->move();
                } else {
                    this->current_match->ticks_to_deadline--;
                }
                cpSpaceStep(this->space, 0.016);
            }
        }

        if (this->current_match->is_match_ended()) {
            LOG(INFO) << "----------------------------------match ended";
            string input_string = "";

            getline(cin, input_string);
            auto l_config = nlohmann::json::parse(input_string);

            if (l_config["type"].get<std::string>() == "new_match") {
                this->config = l_config;
            }

            this->next_match();
        }

        this->tick_num++;
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

    void next_match() {

        clear_spaces();
        this->current_match = new Match(this->config, this->players, this->space, true);
        this->match_objects  = this->current_match->get_object_for_space();

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


    static void addBodyToSpace(cpSpace * space, cpBody * body) {

        //if (body->space != space) {
            cpSpaceAddBody(space, body);
        //}
    }

    static void addShapeToSpace(cpSpace * space, cpShape * shape) {

        //if (shape->space != space) {
            cpSpaceAddShape(space, shape);
        //}
    }

    static void addConstraitToSpace(cpSpace * space, cpConstraint * constraint) {

        //if (constraint->space != space) {
            cpSpaceAddConstraint(space, constraint);
        //}
    }
};


#endif //MADCARS_GAME_H
