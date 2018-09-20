//
// Created by lookuut on 14.09.18.
//

#ifndef MADCARS_CARSTATE_H
#define MADCARS_CARSTATE_H


#include <chipmunk/cpVect.h>
#include "../models/Car.h"
#include <math.h>

class CarState {
public:

    cpVect body_pos;
    double body_angle;

    cpVect front_wheel_pos;
    double front_wheel_angle;

    cpVect rear_wheel_pos;
    double rear_wheel_angle;

    vector<cpVect> * button_shape;
public:

    CarState(json & car_state) {
        body_pos = cpv(car_state[0][0].get<double>(), car_state[0][1].get<double>());
        body_angle = car_state[1].get<double>();

        rear_wheel_pos = cpv(car_state[3][0].get<double>(), car_state[3][1].get<double>());
        rear_wheel_angle = car_state[3][2].get<double>();

        front_wheel_pos = cpv(car_state[4][0].get<double>(), car_state[4][1].get<double>());
        front_wheel_angle = car_state[4][2].get<double>();

        this->button_shape = NULL;
    }


    ~CarState() {
        delete this->button_shape;
    }

    CarState(const CarState &state) {
        body_pos = state.body_pos;
        body_angle = state.body_angle;

        rear_wheel_pos = state.rear_wheel_pos;
        rear_wheel_angle = state.rear_wheel_angle;

        front_wheel_pos = state.front_wheel_pos;
        front_wheel_angle = state.front_wheel_angle;

        *button_shape = *(state.button_shape);
    }

    CarState(Car * car) {
        body_pos = cpBodyGetPosition(car->car_body);
        body_angle = cpBodyGetAngle(car->car_body);

        rear_wheel_pos = cpBodyGetPosition(car->rear_wheel_body);
        rear_wheel_angle = cpBodyGetAngle(car->rear_wheel_body);

        front_wheel_pos = cpBodyGetPosition(car->front_wheel_body);
        front_wheel_angle = cpBodyGetAngle(car->front_wheel_body);

        this->button_shape = car->get_button_world_coors();
    }

    bool is_equal (Car * car) {
        cpVect l_body_vec = cpBodyGetPosition(car->car_body);
        double l_body_angle = cpBodyGetAngle(car->car_body);

        cpVect l_rear_wheel_body_vec = cpBodyGetPosition(car->rear_wheel_body);
        double l_rear_wheel_body_angle = cpBodyGetAngle(car->rear_wheel_body);

        cpVect l_front_wheel_body_vec = cpBodyGetPosition(car->front_wheel_body);
        double l_front_wheel_body_angle = cpBodyGetAngle(car->front_wheel_body);

        double delta = abs(body_pos.x - l_body_vec.x) + abs(body_pos.y - l_body_vec.y) + abs(body_angle - l_body_angle);
        delta += abs(rear_wheel_pos.x - l_rear_wheel_body_vec.x) + abs(rear_wheel_pos.y - l_rear_wheel_body_vec.y) + abs(rear_wheel_angle - l_rear_wheel_body_angle);
        delta += abs(front_wheel_pos.x - l_front_wheel_body_vec.x) + abs(front_wheel_pos.y - l_front_wheel_body_vec.y) + abs(front_wheel_angle - l_front_wheel_body_angle);

        return delta <= 0.;
    }

    string to_string() {
        return std::to_string(body_pos.x) + " " + std::to_string(body_pos.y) + " " + std::to_string(body_angle);
    }

    vector<cpVect> * get_button_shape() {
        return this->button_shape;
    }

    static cpVect line_intersect(cpVect fLineStart, cpVect fLineEnd, cpVect sLineStart, cpVect sLineEnd) {

        cpVect dir1 = cpvsub(fLineEnd, fLineStart);
        cpVect dir2 = cpvsub(sLineEnd, sLineStart);

        //считаем уравнения прямых проходящих через отрезки
        double a1 = -dir1.y;
        double b1 = +dir1.x;
        double d1 = -(a1 * fLineStart.x + b1 * fLineStart.y);

        double a2 = -dir2.y;
        double b2 = +dir2.x;
        double d2 = -(a2 * sLineStart.x + b2 * sLineStart.y);

        //подставляем концы отрезков, для выяснения в каких полуплоскотях они
        double seg1_line2_start = a2 * fLineStart.x + b2 * fLineStart.y + d2;
        double seg1_line2_end = a2 * fLineEnd.x + b2 * fLineEnd.y + d2;

        double seg2_line1_start = a1*sLineStart.x + b1*sLineStart.y + d1;
        double seg2_line1_end = a1*sLineEnd.x + b1*sLineEnd.y + d1;

        //если концы одного отрезка имеют один знак, значит он в одной полуплоскости и пересечения нет.
        if (seg1_line2_start * seg1_line2_end > 0 || seg2_line1_start * seg2_line1_end > 0) {
            return cpv(0, 0);
        }

        double u = seg1_line2_start / (seg1_line2_start - seg1_line2_end);
        return cpvadd(fLineStart, cpvmult(dir1, u));

    }
};


#endif //MADCARS_CARSTATE_H
