//
// Created by lookuut on 09.09.18.
//

#include "Deadline.h"

Deadline::Deadline(short type, double max_lenght, double max_height) {
    this->type = type;

    cpBody * body = cpBodyNewKinematic();
    this->line = cpSegmentShapeNew(body, cpv(0, 0), cpv(max_lenght, 0), 2);
    cpShapeSetSensor(this->line, true);
    cpBodySetPosition(this->line->body, cpv(0,  ( type == ASC ? 10  : max_height - 10)));
}

void Deadline::move() {

    cpVect pos = cpBodyGetPosition(this->line->body);

    if (this->type == ASC) {
        cpBodySetPosition(this->line->body, cpv(pos.x, pos.y + 0.5));
    } else {
        cpBodySetPosition(this->line->body, cpv(pos.x, pos.y - 0.5));
    }
}

cpShape * Deadline::get_object_for_space() {
    return this->line;
}

double Deadline::get_position() {
    return cpBodyGetPosition(this->line->body).y;
}

