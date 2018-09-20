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
    CarState * my_state;
    CarState * enemy_state;
    int tick = 0;
    vector<int> * step_sizes;

    SimVariance(CarState * state, CarState * enemy_state, std::list<short> steps, int tick, vector<int> * step_sizes){
        this->my_state = state;
        this->enemy_state = enemy_state;

        this->steps = steps;
        this->tick = tick;
        this->step_sizes = step_sizes;
    }

    ~SimVariance() {
        delete my_state;
        delete enemy_state;
    }

    /**
     * is given state better then current
     */
    bool is_weakness(int c_tick, Car *car, Car * enemy_car) {

        if (this->tick > c_tick) {
            return true;
        } else if (this->tick < c_tick) {
            return false;
        }

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

        return (c_tick >= Constants::TICKS_TO_DEADLINE ? min_y->y < c_min_y->y : distance > c_distance);
    }

    list<short> get_steps() {
        list<short> future_steps;
        int step_size_pos = 0;

        for (std::list<short>::iterator it = steps.begin(); it != steps.end(); ++it, step_size_pos++) {

            for (int i = 0; i < (*step_sizes)[step_size_pos]; i++) {
                future_steps.push_back(*it);
            }


        }

        return future_steps;
    }
};


#endif //MADCARS_SIMVARIANCE_H
