//
// Created by lookuut on 09.09.18.
//

#include "Deadline.h"

Deadline::Deadline(short type, double max_lenght, double max_height) {
    this->type = type;

    cpBody * body = cpBodyNewKinematic();

    cpVect * polyVect = new cpVect[4];

    polyVect[0] = cpv(0., 2.);
    polyVect[1] = cpv(max_lenght, 2.);
    polyVect[2] = cpv(max_lenght, -Deadline::deadline_height);
    polyVect[3] = cpv(0., -Deadline::deadline_height);

    this->line = cpPolyShapeNew(body, 4, polyVect, cpTransformNew(1, 0, 0, 1, 0, 0), 0.);

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

