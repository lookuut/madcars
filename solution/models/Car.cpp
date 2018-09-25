//
// Created by lookuut on 09.09.18.
//


#include "Car.h"
#include "../utils/ShapeCounter.h"
#include <utility>"
#include <iostream>

Car::Car(const json & car_config, int car_group, int modification, cpSpace *space) {
    this->motors = list<cpConstraint*>();
    this->space = space;
    this->car_group = car_group;
    this->x_modification = modification;

    this->car_body_poly = car_config["car_body_poly"].get<std::vector<vector<double>>>();
    this->car_body_mass = car_config["car_body_mass"].get<double>();

    this->car_body_friction = car_config["car_body_friction"].get<double>();
    this->car_body_elasticity = car_config["car_body_elasticity"].get<double>();

    this->button_poly = car_config["button_poly"].get<std::vector<vector<double>>>();

    this->max_speed = car_config["max_speed"].get<double>();
    this->max_angular_speed = car_config["max_angular_speed"].get<double>();
    this->torque = car_config["torque"].get<double>();
    this->drive = car_config["drive"].get<short>();

    this->rear_wheel_mass = car_config["rear_wheel_mass"].get<double>();
    this->rear_wheel_radius = car_config["rear_wheel_radius"].get<double>();
    this->rear_wheel_position = cpv(car_config["rear_wheel_position"][0].get<double>(), car_config["rear_wheel_position"][1].get<double>());
    this->rear_wheel_friction = car_config["rear_wheel_friction"].get<double>();
    this->rear_wheel_elasticity = car_config["rear_wheel_elasticity"].get<double>();
    this->rear_wheel_joint = (car_config["rear_wheel_joint"][0].get<double>(), car_config["rear_wheel_joint"][1].get<double>());
    this->rear_wheel_groove_offset = car_config["rear_wheel_groove_offset"].get<double>();
    this->rear_wheel_damp_position = cpv(
            car_config["rear_wheel_damp_position"][0].get<double>(), car_config["rear_wheel_damp_position"][1].get<double>()
    );

    this->rear_wheel_damp_length = car_config["rear_wheel_damp_length"].get<double>();
    this->rear_wheel_damp_stiffness = car_config["rear_wheel_damp_stiffness"].get<double>();
    this->rear_wheel_damp_damping = car_config["rear_wheel_damp_damping"].get<double>();

    this->front_wheel_mass = car_config["front_wheel_mass"].get<double>();
    this->front_wheel_radius = car_config["front_wheel_radius"].get<double>();
    this->front_wheel_position = cpv(car_config["front_wheel_position"][0].get<double>(), car_config["front_wheel_position"][1].get<double>());
    this->front_wheel_friction = car_config["front_wheel_friction"].get<double>();
    this->front_wheel_elasticity = car_config["front_wheel_elasticity"].get<double>();
    this->front_wheel_joint = cpv(car_config["front_wheel_joint"][0].get<double>(), car_config["front_wheel_joint"][1].get<double>());
    this->front_wheel_groove_offset = car_config["front_wheel_groove_offset"].get<double>();

    this->front_wheel_damp_position = cpv(
            car_config["front_wheel_damp_position"][0].get<double>(), car_config["front_wheel_damp_position"][1].get<double>()
    );

    this->front_wheel_damp_length = car_config["front_wheel_damp_length"].get<double>();
    this->front_wheel_damp_stiffness = car_config["front_wheel_damp_stiffness"].get<double>();
    this->front_wheel_damp_damping = car_config["front_wheel_damp_damping"].get<double>();

    this->button_collision_type = car_group * 10;

    this->car_body = create_car_body();
    this->car_shape = create_car_shape();
    this->button_shape = create_button_shape();


    cpBodySetCenterOfGravity(this->car_body, cpShapeGetCenterOfGravity(this->car_shape));

    this->rear_wheel = (car_config["external_id"].get<int>() == 3 ? create_square_wheel("rear") : create_wheel("rear"));
    this->front_wheel = (car_config["external_id"].get<int>() == 3 ? create_square_wheel("front") : create_wheel("front"));

    this->front_wheel_body = std::get<cpBody*>(this->front_wheel);
    this->rear_wheel_body = std::get<cpBody*>(this->rear_wheel);

    this->rear_wheel_motor = std::get<cpConstraint*>(this->rear_wheel);
    this->front_wheel_motor = std::get<cpConstraint*>(this->front_wheel);

    this->rear_wheel_objects = std::get<car_wheel_objects*>(this->rear_wheel);
    this->front_wheel_objects = std::get<car_wheel_objects*>(this->front_wheel);

    if (this->rear_wheel_motor != NULL) {
        this->motors.push_back(this->rear_wheel_motor);
    }

    if (this->front_wheel_motor != NULL) {
        this->motors.push_back(this->front_wheel_motor);
    }
}

std::tuple<cpBody *, cpConstraint *, car_wheel_objects*> Car::create_wheel(std::string wheel_side) {

    car_wheel_objects * wheel_objects = new car_wheel_objects;


    double wheel_mass = (wheel_side == "rear" ? this->rear_wheel_mass : this->front_wheel_mass);
    double wheel_radius = (wheel_side == "rear" ? this->rear_wheel_radius : this->front_wheel_radius);
    cpVect wheel_position = (wheel_side == "rear" ? this->rear_wheel_position : this->front_wheel_position);
    double wheel_friction = (wheel_side == "rear" ? this->rear_wheel_friction : this->front_wheel_friction);
    double wheel_elasticity = (wheel_side == "rear" ? this->rear_wheel_elasticity : this->front_wheel_elasticity);
    double wheel_groove_offset = (wheel_side == "rear" ? this->rear_wheel_groove_offset : this->front_wheel_groove_offset);
    cpVect wheel_damp_position = (wheel_side == "rear" ? this->rear_wheel_damp_position : this->front_wheel_damp_position);
    double wheel_damp_length = (wheel_side == "rear" ? this->rear_wheel_damp_length : this->front_wheel_damp_length);
    double wheel_damp_stiffness = (wheel_side == "rear" ? this->rear_wheel_damp_stiffness : this->front_wheel_damp_stiffness);
    double wheel_damp_damping = (wheel_side == "rear" ? this->rear_wheel_damp_damping : this->front_wheel_damp_damping);

    cpBody * wheel_body = cpBodyNew(wheel_mass, cpMomentForCircle(wheel_mass, 0 , wheel_radius, cpv(0, 0)));
    cpBodySetPosition(wheel_body, cpv(wheel_position.x * this->x_modification, wheel_position.y));

    wheel_objects->wheel_shape = cpCircleShapeNew(wheel_body, wheel_radius, cpv(0, 0));
    cpShapeSetUserData(wheel_objects->wheel_shape, (cpDataPointer)ShapeCounter::getInstance().operator++());

    cpShapeSetFilter(wheel_objects->wheel_shape, cpShapeFilterNew(this->car_group, 4294967295, 4294967295));

    cpShapeSetFriction(wheel_objects->wheel_shape, wheel_friction);
    cpShapeSetElasticity(wheel_objects->wheel_shape, wheel_elasticity);

    wheel_objects->wheel_groove = cpGrooveJointNew(this->car_body, wheel_body,
                                                   cpv(
                                                           wheel_damp_position.x * this->x_modification,
                                                           wheel_damp_position.y  - wheel_groove_offset
                                                   ),
                                                   cpv(
                                                           wheel_damp_position.x * this->x_modification,
                                                           wheel_damp_position.y - wheel_damp_length * 1.5
                                                   ),
                                                   cpv(0,0)
                                                   );

    wheel_objects->wheel_damp = cpDampedSpringNew(
             wheel_body,
             this->car_body,
             cpv(0,0),
             cpv(wheel_damp_position.x * this->x_modification, wheel_damp_position.y),
             wheel_damp_length,
             wheel_damp_stiffness,
             wheel_damp_damping
             );

    cpConstraint * wheel_motor = NULL;

    if  (
        (wheel_side == "rear" && (this->drive == Car::AWD ||  this->drive == Car::FR))
        ||
        (wheel_side == "front" && (this->drive == Car::AWD ||  this->drive == Car::FF))
        )
    {
        wheel_motor = cpSimpleMotorNew(wheel_body, this->car_body, 0);
    }

    return std::make_tuple(wheel_body, wheel_motor, wheel_objects);
}


std::tuple<cpBody *, cpConstraint *, car_wheel_objects *> Car::create_square_wheel(std::string wheel_side) {
    car_wheel_objects * wheel_objects = new car_wheel_objects;

    double wheel_mass = (wheel_side == "rear" ? this->rear_wheel_mass : this->front_wheel_mass);
    double wheel_radius = (wheel_side == "rear" ? this->rear_wheel_radius : this->front_wheel_radius);
    cpVect wheel_position = (wheel_side == "rear" ? this->rear_wheel_position : this->front_wheel_position);
    double wheel_friction = (wheel_side == "rear" ? this->rear_wheel_friction : this->front_wheel_friction);
    double wheel_elasticity = (wheel_side == "rear" ? this->rear_wheel_elasticity : this->front_wheel_elasticity);
    double wheel_groove_offset = (wheel_side == "rear" ? this->rear_wheel_groove_offset : this->front_wheel_groove_offset);
    cpVect wheel_damp_position = (wheel_side == "rear" ? this->rear_wheel_damp_position : this->front_wheel_damp_position);
    double wheel_damp_length = (wheel_side == "rear" ? this->rear_wheel_damp_length : this->front_wheel_damp_length);
    double wheel_damp_stiffness = (wheel_side == "rear" ? this->rear_wheel_damp_stiffness : this->front_wheel_damp_stiffness);
    double wheel_damp_damping = (wheel_side == "rear" ? this->rear_wheel_damp_damping : this->front_wheel_damp_damping);

    cpBody * wheel_body = cpBodyNew(wheel_mass, cpMomentForBox(wheel_mass, wheel_radius * 2.0, wheel_radius * 2.0));
    cpBodySetPosition(wheel_body, cpv(wheel_position.x * this->x_modification, wheel_position.y));

    wheel_objects->wheel_shape = cpBoxShapeNew(wheel_body, wheel_radius * 2.0, wheel_radius * 2.0, 0);
    cpShapeSetFilter(wheel_objects->wheel_shape, cpShapeFilterNew(this->car_group, 4294967295, 4294967295));

    cpShapeSetFriction(wheel_objects->wheel_shape, wheel_friction);
    cpShapeSetElasticity(wheel_objects->wheel_shape, wheel_elasticity);

    wheel_objects->wheel_groove = cpGrooveJointNew(this->car_body, wheel_body,
                                                   cpv(
                                                           wheel_damp_position.x * this->x_modification,
                                                           wheel_damp_position.y  - wheel_groove_offset
                                                   ),
                                                   cpv(
                                                           wheel_damp_position.x * this->x_modification,
                                                           wheel_damp_position.y - wheel_damp_length * 1.5
                                                   ),
                                                   cpv(0, 0)
    );

    wheel_objects->wheel_damp = cpDampedSpringNew(
            wheel_body,
            this->car_body,
            cpv(0,0),
            cpv(wheel_damp_position.x * this->x_modification, wheel_damp_position.y),
            wheel_damp_length,
            wheel_damp_stiffness,
            wheel_damp_damping
    );

    cpConstraint * wheel_motor = NULL;

    if (
            wheel_side == "rear" && (this->drive == Car::AWD ||  this->drive == Car::FR)
            ||
            wheel_side == "front" && (this->drive == Car::AWD ||  this->drive == Car::FF))
    {
        wheel_motor = cpSimpleMotorNew(wheel_body, this->car_body, 0);
    }

    return std::make_tuple(wheel_body, wheel_motor, wheel_objects);
}

//@TODO fix memory leak
cpVect * Car::processed_poly(vector<vector<double>> polygon) {

    cpVect * vectors = new cpVect[polygon.size()];

    int pos = 0;
    for (vector<vector<double>>::iterator it = polygon.begin(); it != polygon.end(); ++it) {
        vectors[pos] = cpv((*it).at(0) * this->x_modification, (*it).at(1));
        pos++;
    }

    return vectors;
}

cpBody * Car::create_car_body() {
    cpVect * vect = this->processed_poly(this->car_body_poly);
    cpBody * body = cpBodyNew(this->car_body_mass, cpMomentForPoly(this->car_body_mass, this->car_body_poly.size(), vect, cpv(0,0), 0));

    delete vect;

    return body;
}


cpShape* Car::create_car_shape() {

    cpVect * vect = this->processed_poly(this->car_body_poly);
    cpShape * shape = cpPolyShapeNew(this->car_body, (int)this->car_body_poly.size(), vect, cpTransformNew(1,0,0,1,0,0), 0);
    delete vect;
    cpShapeSetFriction(shape, this->car_body_friction);
    cpShapeSetElasticity(shape, this->car_body_elasticity);
    cpShapeSetFilter(shape, cpShapeFilterNew(this->car_group, 4294967295, 4294967295));

    return shape;
}

cpShape * Car::create_button_shape() {

    cpVect * vect = this->processed_poly(this->button_poly);
    cpShape * shape = cpPolyShapeNew(this->car_body, (int)this->button_poly.size(), vect, cpTransformNew(1,0,0,1,0,0), 0);
    delete vect;

    cpShapeSetFilter(shape, cpShapeFilterNew(this->car_group, 4294967295, 4294967295));
    cpShapeSetSensor(shape, true);
    cpShapeSetCollisionType(shape, this->button_collision_type);

    return shape;
}

car_objects_type * Car::get_objects_for_space_at(cpVect point) {

    cpBodySetPosition(this->car_body, point);
    cpBodySetPosition(this->front_wheel_body, cpv(point.x + this->front_wheel_position.x * this->x_modification, point.y + this->front_wheel_position.y ));
    cpBodySetPosition(this->rear_wheel_body, cpv(point.x + this->rear_wheel_position.x * this->x_modification, point.y + this->rear_wheel_position.y));

    car_objects_type * objects = new car_objects_type();

    objects->button_shape = this->button_shape;
    objects->car_body = this->car_body;
    objects->car_shape = this->car_shape;
    objects->rear_wheel_body = this->rear_wheel_body;
    objects->front_wheel_body = this->front_wheel_body;
    objects->rear_wheel = this->rear_wheel_objects;
    objects->front_wheel = this->front_wheel_objects;
    objects->motors = this->motors;

    return objects;
}


cpShape * Car::get_button_poly() {
    return this->button_shape;
}

cpVect Car::to_world(const cpVect & vect) {

    cpTransform transform = this->car_body->transform;

    return cpTransformPoint(transform , vect);
}

vector<cpVect> Car::get_button_world_coors() {
    vector<cpVect> transformed = vector<cpVect>();

    for (vector<vector<double>>::iterator it = button_poly.begin(); it != button_poly.end(); ++it) {
        transformed.push_back(to_world(cpv((*it).at(0), (*it).at(1))) );
    }

    return transformed;
}
