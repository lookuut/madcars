//
// Created by lookuut on 25.09.18.
//

#ifndef MADCARS_EVALUATEFUNC_H
#define MADCARS_EVALUATEFUNC_H


#include "CarState.h"

class EvaluateFunc {
private :

    list<short> best_decision_commands;
    list<CarState> best_decision_states;

    double best_decision_height;
    int min_tick = INT32_MAX;
    char is_win = -1;
public:

    EvaluateFunc() {
    }


    void evaluate(int tick, char is_win, const CarState state, const list<short> commands, const list<CarState> states) {

        if (is_win < this->is_win) {
            return;
        }

        vector<cpVect> button_shape = state.get_button_shape();
        auto min_point = std::min_element( button_shape.begin(), button_shape.end(),
                                           []( const cpVect &a, const cpVect &b )
                                           {
                                               return a.y > b.y;
                                           } );

        this->is_win = is_win;

        if (is_win == 1 && tick < min_tick) {
            min_tick = tick;
            best_decision_commands = commands;
            best_decision_states = states;
            best_decision_height = min_point->y;
        } else if (is_win == 0 && min_point->y > best_decision_height) {
            best_decision_commands = commands;
            best_decision_states = states;
            best_decision_height = min_point->y;
        } else if (is_win == -1) {
            best_decision_commands = commands;
            best_decision_states = states;
            best_decision_height = min_point->y;
            this->is_win = 0;
        }
    }

    const list<short> get_commands() {
        return best_decision_commands;
    }

    const list<CarState> get_states() {
        return best_decision_states;
    }
};

#endif //MADCARS_EVALUATEFUNC_H
