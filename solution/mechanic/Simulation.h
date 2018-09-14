//
// Created by lookuut on 13.09.18.
//

#ifndef MADCARS_SIMULATION_H
#define MADCARS_SIMULATION_H


#include <chipmunk/chipmunk_structs.h>

class Simulation {

public:

    Simulation(cpSpace * space) {

    }


    static cpSpace * copy_space(cpSpace * space) {

        cpSpaceAlloc();
    }
};


#endif //MADCARS_SIMULATION_H
