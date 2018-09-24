//
// Created by lookuut on 15.09.18.
//

#ifndef MADCARS_SIMVARIANCE_H
#define MADCARS_SIMVARIANCE_H

#include <list>
#include <chipmunk/cpVect.h>
#include "CarState.h"
#include "../models/Constans.h"

class SimVariance {
public :

    std::list<short>  steps;
    std::list<CarState*> * step_states;

    CarState * my_state;
    CarState * enemy_state;
    int tick = 0;
    vector<int> * step_sizes;

    SimVariance(CarState * state, CarState * enemy_state, std::list<short> steps, int tick, vector<int> * step_sizes, list<CarState*> * step_states){
        this->my_state = state;
        this->enemy_state = enemy_state;

        this->steps = steps;
        this->tick = tick;
        this->step_sizes = step_sizes;

        this->step_states = new list<CarState*>();

        for (std::list<CarState*>::iterator it = step_states->begin(); it != step_states->end(); ++it) {
            if (*it != NULL) {
                this->step_states->push_back( new CarState( **it ) );
            }
        }
    }

    ~SimVariance() {
        delete my_state;
        delete enemy_state;
        step_states->remove_if(deleteAll);
        delete step_states;
    }

    static bool deleteAll( CarState * theElement ) { delete theElement; return true; }

    /**
     * is given state better then current
     */
    bool is_weakness(int c_tick, Car *car, Car * enemy_car) {

        /*
        if (this->tick > c_tick) {
            return true;
        } else if (this->tick < c_tick) {
            return false;
        }*/

        bool is_planning = car->is_planning();


        //remove this
        vector<cpVect> * c_button_coors = car->get_button_world_coors();
        vector<cpVect> * button_coors = my_state->get_button_shape();

        cpAssertHard(button_coors != NULL, "button coors is null at sim variance");

        //temporary boolshit

        auto c_min_y = std::min_element( c_button_coors->begin(), c_button_coors->end(),
                                     []( const cpVect &a, const cpVect &b )
                                     {
                                         return a.y > b.y;
                                     } );

        auto min_y = std::min_element( button_coors->begin(), button_coors->end(),
                                           []( const cpVect &a, const cpVect &b )
                                           {
                                               return a.y > b.y;
                                           } );

        delete c_button_coors;

        double c_distance = cpvlength( cpvsub(cpBodyGetPosition(car->car_body), cpBodyGetPosition(enemy_car->car_body)));
        double distance = cpvlength( cpvsub(my_state->body_pos, enemy_state->body_pos));

        return min_y->y < c_min_y->y;
        //return (c_tick >= Constants::TICKS_TO_DEADLINE ? min_y->y < c_min_y->y : distance > c_distance);
    }

    list<short> get_steps() {
        return steps;
    }

    list<CarState*> * get_states() {
        return this->step_states;
    }
};


#endif //MADCARS_SIMVARIANCE_H
