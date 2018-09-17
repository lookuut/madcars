//
// Created by lookuut on 10.09.18.
//

#ifndef MADCARS_PLAYER_H
#define MADCARS_PLAYER_H


#include <iostream>
#include "../models/Car.h"
#include <stdlib.h>     /* srand, rand */
#include <deque>

using namespace std;

class Player {
private:

    int id;
    int lives;
    Car * car;

    bool is_dead_at_match = false;
    bool is_original;
    deque<short> command_queue;
public:

    bool is_ally;

    static const char* const commands[];
    static const int max_command = 2;

    Player(int id, int lives, bool is_ally, bool is_original) {
        this->id = id;
        this->lives = lives;
        this->is_ally = is_ally;
        this->is_original = is_original;
        this->car = NULL;

        srand(time(NULL));
    }

    ~Player() {
        delete car;
    }

    void set_car(Car * car) {
        this->car = car;
    }

    void apply_turn(int tick) {

        short step = pop_command();

        switch (step) {
            case 0:
                this->get_car()->go_left();
                break;
            case 1:
                this->get_car()->go_right();
                break;
            case 2:
                this->get_car()->stop();
                break;
        }
    }

    Car * get_car() {
        return this->car;
    }

    int get_game_id() {
        this->id;
    }

    int die() {
        this->lives -= 1;
    }


    bool is_alive() {
        return this->lives > 0;
    }

    int get_lives() {
        return this->lives;
    }

    void push_command(short command) {
        command_queue.push_back(command);
    }

    short pop_command() {
        cpAssertHard(command_queue.size() != 0, "Queue is empty");

        short command = command_queue.front();
        command_queue.pop_front();

        return command;
    }

    void clear_command_queue() {
        command_queue = deque<short>();
    }

    void set_dead(bool state) {
        this->is_dead_at_match = state;
    }

    bool is_dead() {
        return this->is_dead_at_match;
    }

    size_t command_queue_size() {
        return this->command_queue.size();
    }
};

#endif //MADCARS_PLAYER_H
