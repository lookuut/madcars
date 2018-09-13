//
// Created by lookuut on 10.09.18.
//

#ifndef MADCARS_PLAYER_H
#define MADCARS_PLAYER_H


#include <iostream>
#include "../models/Car.h"
#include <stdlib.h>     /* srand, rand */

using namespace std;

class Player {
private:

    int id;
    int lives;
    Car * car;

    bool is_original;

public:

    bool is_ally;
    string commands[3] = {"right", "left", "stop"};

    Player(int id, int lives, bool is_ally, bool is_original) {
        this->id = id;
        this->lives = lives;
        this->is_ally = is_ally;
        this->is_original = is_original;
        this->car = NULL;

        srand (time(NULL));
    }

    void set_car(Car * car) {
        this->car = car;
    }

    void apply_turn(int tick) {
        if (this->is_ally) {

            this->car->go_left();

            /*
            int pos = rand() % 3;
            switch(pos) {

                case 1 : this->car->go_right();
                case 2 : this->car->stop();
            }*/

            if (this->is_original) {
                nlohmann::json command;
                //command["command"] = commands[pos];
                //command["debug"] = commands[pos];
                command["command"] = "left";
                command["debug"] = "left";
                cout << command.dump() << endl;
            }
        } else {
            this->car->stop();
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
};


#endif //MADCARS_PLAYER_H
