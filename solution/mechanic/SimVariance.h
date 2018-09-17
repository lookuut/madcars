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
    CarState * state;
    int tick = 0;

    SimVariance(CarState * state, std::list<short> steps, int tick){
        this->state = state;
        this->steps = steps;
        this->tick = tick;
    }

    ~SimVariance() {
        delete state;
    }

    /**
     * is given state better then current
     */
    bool is_weakness(int c_tick, Car *car) {

        if (this->tick > c_tick) {
            return true;
        } else if (this->tick < c_tick) {
            return false;
        }

        cpVect velocity = cpBodyGetVelocity(car->car_body);


        //remove this
        vector<cpVect> * c_button_coors = car->get_button_world_coors();
        vector<cpVect> * button_coors = state->get_button_shape();

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

        return min_y->y < c_min_y->y;
    }

    list<short> get_steps() {
        list<short> future_steps;

        for (std::list<short>::iterator it = steps.begin(); it != steps.end(); ++it) {

            for (int i = 0; i < Constants::SIMULATION_STEP_SIZE; i++) {
                future_steps.push_back(*it);
            }
        }

        return future_steps;
    }
};


#endif //MADCARS_SIMVARIANCE_H
