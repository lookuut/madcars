//
// Created by lookuut on 09.09.18.
//

#ifndef MADCARS_DEADLINE_H
#define MADCARS_DEADLINE_H

#include "../../chipmunk_src/include/chipmunk/chipmunk_private.h"


#define ASC 1
#define DESC 2

class Deadline {

private:
    cpShape * line;
    short type;
public:
    Deadline();
    Deadline(short type, double max_lenght, double max_height);
    void move();

    cpShape * get_object_for_space();

    double get_position();

    static constexpr double deadline_height = 20;

};


#endif //MADCARS_DEADLINE_H
