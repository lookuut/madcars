//
// Created by lookuut on 09.09.18.
//

#ifndef MADCARS_CAR_H
#define MADCARS_CAR_H


#include <list>

#include <chipmunk/chipmunk_structs.h>
#include <iostream>
#include "../../../nlohmann/json.hpp"

#include "../utils/easylogging++.h"


using namespace nlohmann;
using namespace std;

struct car_wheel_objects {
    cpShape * wheel_shape;
    cpConstraint * wheel_groove;
    cpConstraint * wheel_damp;
};
typedef struct car_wheel_objects car_wheel_objects;

struct car_objects_type {

    cpShape * button_shape;
    cpBody * car_body;
    cpShape * car_shape;
    cpBody * rear_wheel_body;
    cpBody * front_wheel_body;
    car_wheel_objects* rear_wheel;
    car_wheel_objects* front_wheel;
    std::list<cpConstraint * > motors;
};
typedef struct car_objects_type car_objects_type;


class Car {
private:

    cpSpace * space;
    int car_group;
    int x_modification;

    cpShape * car_shape;
    cpShape * button_shape;

    cpConstraint * rear_wheel_motor = NULL;
    cpConstraint * front_wheel_motor = NULL;


    vector<vector<double>>car_body_poly;
    vector<vector<double>>button_poly;

    std::list<cpConstraint*> motors;

    double car_body_mass;

    double car_body_friction;

    double car_body_elasticity;

    double max_speed;
    double max_angular_speed;
    double torque;
    short drive;


    std::tuple<cpBody *, cpConstraint *, car_wheel_objects *> rear_wheel;
    std::tuple<cpBody *, cpConstraint *, car_wheel_objects *> front_wheel;
    cpBody * rear_wheel_body;
    double rear_wheel_mass;
    double rear_wheel_radius;
    cpVect rear_wheel_position;
    double rear_wheel_friction;
    double rear_wheel_elasticity;
    double rear_wheel_joint;
    double rear_wheel_groove_offset;
    cpVect rear_wheel_damp_position;

    double rear_wheel_damp_length;
    double rear_wheel_damp_stiffness;
    double rear_wheel_damp_damping;
    car_wheel_objects * rear_wheel_objects;

    cpBody * front_wheel_body;
    double front_wheel_mass;
    double front_wheel_radius;
    cpVect front_wheel_position;
    double front_wheel_friction;
    double front_wheel_elasticity;
    cpVect front_wheel_joint;
    double front_wheel_groove_offset;

    cpVect front_wheel_damp_position;

    double front_wheel_damp_length;
    double front_wheel_damp_stiffness;
    double front_wheel_damp_damping;
    car_wheel_objects * front_wheel_objects;

    short button_collision_type;
    cpSpacePointQueryFunc * point_query_nearest;

public:
    cpBody * car_body;

    static const short FF = 1;
    static const short FR = 2;
    static const short AWD = 3;

    Car(json & car_config, int car_group, int direction, cpSpace * space);

    ~Car(){
    }

    std::tuple<cpBody *, cpConstraint *, car_wheel_objects *> create_wheel(std::string wheel_side);
    std::tuple<cpBody *, cpConstraint *, car_wheel_objects *> create_square_wheel(std::string wheel_side);

    cpVect * processed_poly(vector<vector<double>> polygon);
    cpBody * create_car_body();
    cpShape * create_car_shape();
    cpShape * create_button_shape();
    cpPolyShape * get_button_poly();
    car_objects_type * get_objects_for_space_at(cpVect  point);

    bool in_air() {

        cpPointQueryInfo * info1 = new cpPointQueryInfo();
        cpPointQueryInfo * info2 = new cpPointQueryInfo();

        cpShape * _shape1 = cpSpacePointQueryNearest(this->space, cpBodyGetPosition(this->rear_wheel_body), this->rear_wheel_radius + 1., cpShapeFilterNew(this->car_group, 0xffffffff, 0xffffffff), info1);
        cpShape * _shape2 = cpSpacePointQueryNearest(this->space, cpBodyGetPosition(this->front_wheel_body), this->front_wheel_radius + 1., cpShapeFilterNew(this->car_group, 0xffffffff, 0xffffffff), info2);

        cpDataPointer shape_pointer1 = NULL;
        if (_shape1 != NULL) {
            shape_pointer1 = cpShapeGetUserData(_shape1);
        }

        cpDataPointer shape_pointer2 = NULL;
        if (_shape2 != NULL) {
            shape_pointer2 = cpShapeGetUserData(_shape2);
        }

        return (bool)!(shape_pointer1 || shape_pointer2);
    }

    void go_right() {

        if (this->in_air()) {
            cpBodySetTorque(this->car_body, this->torque);
        }

        for (std::list<cpConstraint*>::iterator it = motors.begin(); it != motors.end(); ++it) {
            cpSimpleMotorSetRate(*it, -this->max_speed);
        }
    }

    void go_left() {

        if (this->in_air()) {
            cpBodySetTorque(this->car_body, -this->torque);
        }

        for (std::list<cpConstraint*>::iterator it = motors.begin(); it != motors.end(); ++it) {
            cpSimpleMotorSetRate(*it, this->max_speed);
        }
    }

    void stop() {

        for (std::list<cpConstraint*>::iterator it = motors.begin(); it != motors.end(); ++it) {
            cpSimpleMotorSetRate(*it, 0);
        }
    }

    short get_button_collision_type() {
        return this->button_collision_type;
    }
};

#endif //MADCARS_CAR_H
