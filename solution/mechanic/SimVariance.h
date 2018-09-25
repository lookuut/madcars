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

    list<short> get_steps() {
        return steps;
    }

    list<CarState*> * get_states() {
        return this->step_states;
    }
};


#endif //MADCARS_SIMVARIANCE_H
