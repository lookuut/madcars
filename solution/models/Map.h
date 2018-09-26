//
// Created by lookuut on 09.09.18.
//

#ifndef MADCARS_MAP_H
#define MADCARS_MAP_H

#include <list>
#include <iostream>

#include "../../chipmunk_src/include/chipmunk/chipmunk_private.h"
#include "../../../nlohmann/json.hpp"
#include <vector>

using namespace std;
using namespace nlohmann;

class Map {
private:

    std::list<cpShape*> objects;
public:

    static constexpr double friction = 1.;
    static constexpr double elasticity = 0.;
    static constexpr double segment_height = 10.;

    static constexpr double max_width = 1200.;
    static constexpr double max_height = 800.;

    Map(const json &segments, cpSpace *space) {

        create_box(space);

        for (auto it = segments.begin(); it != segments.end(); ++it)
        {
            cpVect a = cpv((*it)[0].get<vector<double>>()[0], (*it)[0].get<vector<double>>()[1]);
            cpVect b = cpv((*it)[1].get<vector<double>>()[0], (*it)[1].get<vector<double>>()[1]);
            double c = (*it)[2].get<double>();

            cpShape* segment = cpSegmentShapeNew(
                    space->staticBody,a, b, c);

            cpShapeSetFriction(segment, Map::friction);
            cpShapeSetElasticity(segment, Map::elasticity);

            this->objects.push_back(segment);
        }
    }

    void create_box(cpSpace * space) {
        double bo = this->segment_height - 1; //box offset

        cpShape * left = cpSegmentShapeNew(space->staticBody, cpv(-bo, -bo), cpv(-bo, this->max_height + bo) , this->segment_height);
        cpShapeSetSensor(left, true);
        this->objects.push_back(left);

        cpShape * top = cpSegmentShapeNew(space->staticBody, cpv(-bo, this->max_height + bo), cpv(this->max_width + bo, this->max_height + bo) , this->segment_height);
        cpShapeSetSensor(top, true);
        this->objects.push_back(top);

        cpShape * right = cpSegmentShapeNew(space->staticBody, cpv(this->max_width + bo, this->max_height + bo), cpv(this->max_width + bo, -bo) , this->segment_height);
        cpShapeSetSensor(right, true);
        this->objects.push_back(right);

        cpShape * bottom = cpSegmentShapeNew(space->staticBody, cpv(this->max_width + bo, -bo), cpv(-bo, -bo) , this->segment_height);
        cpShapeSetSensor(bottom, true);
        this->objects.push_back(bottom);
    }

    std::list<cpShape*> get_objects_for_space() {
        return this->objects;
    }

    ~Map() {
        /* // no need to release cpShapes
        for (std::list<cpShape*>::iterator it = objects.begin(); it != objects.end(); ++it) {
            delete (*it);
        } */

    }
};


#endif //MADCARS_MAP_H
