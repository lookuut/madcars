//
// Created by lookuut on 14.09.18.
//

#ifndef MADCARS_CARSTATE_H
#define MADCARS_CARSTATE_H


#include <chipmunk/cpVect.h>
#include "../models/Car.h"

class CarState {
public:
    double s_body_x;
    double s_body_y;

    double s_body_angle;

    double s_rear_wheel_body_x;
    double s_rear_wheel_body_y;
    double s_rear_wheel_body_angle;

    double s_front_wheel_body_x;
    double s_front_wheel_body_y;
    double s_front_wheel_body_angle;

    vector<cpVect> * button_shape;
public:

    CarState(json & car_state) {
        s_body_x = car_state[0][0].get<double>();
        s_body_y = car_state[0][1].get<double>();

        s_body_angle = car_state[1].get<double>();

        s_rear_wheel_body_x = car_state[3][0].get<double>();
        s_rear_wheel_body_y = car_state[3][1].get<double>();
        s_rear_wheel_body_angle = car_state[3][2].get<double>();

        s_front_wheel_body_x = car_state[4][0].get<double>();
        s_front_wheel_body_y = car_state[4][1].get<double>();
        s_front_wheel_body_angle = car_state[4][2].get<double>();
        this->button_shape = NULL;
    }


    ~CarState() {
        delete this->button_shape;
    }

    CarState(const CarState &state) {
        s_body_x = state.s_body_x;
        s_body_y = state.s_body_y;

        s_body_angle = state.s_body_angle;

        s_rear_wheel_body_x = state.s_rear_wheel_body_x;
        s_rear_wheel_body_y = state.s_rear_wheel_body_y;
        s_rear_wheel_body_angle = state.s_rear_wheel_body_angle;

        s_front_wheel_body_x = state.s_front_wheel_body_x;
        s_front_wheel_body_y = state.s_front_wheel_body_y;
        s_front_wheel_body_angle = state.s_front_wheel_body_angle;
        //deep copy of vector
    }

    CarState(Car * car) {
        cpVect l_body_vec = cpBodyGetPosition(car->car_body);
        double l_body_angle = cpBodyGetAngle(car->car_body);

        cpVect l_rear_wheel_body_vec = cpBodyGetPosition(car->rear_wheel_body);
        double l_rear_wheel_body_angle = cpBodyGetAngle(car->rear_wheel_body);

        cpVect l_front_wheel_body_vec = cpBodyGetPosition(car->front_wheel_body);
        double l_front_wheel_body_angle = cpBodyGetAngle(car->front_wheel_body);

        s_body_x = l_body_vec.x;
        s_body_y = l_body_vec.y;

        s_body_angle = l_body_angle;

        s_rear_wheel_body_x = l_rear_wheel_body_vec.x;
        s_rear_wheel_body_y = l_rear_wheel_body_vec.y;
        s_rear_wheel_body_angle = l_rear_wheel_body_angle;

        s_front_wheel_body_x = l_front_wheel_body_vec.x;
        s_front_wheel_body_y = l_front_wheel_body_vec.y;
        s_front_wheel_body_angle = l_front_wheel_body_angle;
        this->button_shape = car->get_button_world_coors();
    }

    bool is_equal (Car * car) {
        cpVect l_body_vec = cpBodyGetPosition(car->car_body);
        double l_body_angle = cpBodyGetAngle(car->car_body);

        cpVect l_rear_wheel_body_vec = cpBodyGetPosition(car->rear_wheel_body);
        double l_rear_wheel_body_angle = cpBodyGetAngle(car->rear_wheel_body);

        cpVect l_front_wheel_body_vec = cpBodyGetPosition(car->front_wheel_body);
        double l_front_wheel_body_angle = cpBodyGetAngle(car->front_wheel_body);

        double delta = abs(s_body_x - l_body_vec.x) + abs(s_body_y - l_body_vec.y) + abs(s_body_angle - l_body_angle);
        delta += abs(s_rear_wheel_body_x - l_rear_wheel_body_vec.x) + abs(s_rear_wheel_body_y - l_rear_wheel_body_vec.y) + abs(s_rear_wheel_body_angle - l_rear_wheel_body_angle);
        delta += abs(s_front_wheel_body_x - l_front_wheel_body_vec.x) + abs(s_front_wheel_body_y - l_front_wheel_body_vec.y) + abs(s_front_wheel_body_angle - l_front_wheel_body_angle);

        return delta <= 0.;
    }

    double get_incicline() {
        return abs(this->s_front_wheel_body_y - this->s_rear_wheel_body_y);
    }

    vector<cpVect> * get_button_shape() {
        return this->button_shape;
    }

};


#endif //MADCARS_CARSTATE_H
