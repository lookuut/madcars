//
// Created by lookuut on 25.09.18.
//

#ifndef MADCARS_EVALUATEFUNC_H
#define MADCARS_EVALUATEFUNC_H

#include "Match.h"
#include "CarState.h"

class EvaluateFunc {
private :

    list<short> best_decision_commands;

    double best_decision_height;
    int min_tick = INT32_MAX;
    char is_win = -1;
    Match * match;
public:

    EvaluateFunc() {

    }

    EvaluateFunc(Match * match) {
        this->match = match;
    }


    void evaluate(int tick, char is_win, const CarState state, const list<short> commands) {


        if (is_win < this->is_win) {
            return;
        }

        vector<cpVect> button_shape = state.get_button_shape();
        auto min_point = std::min_element( button_shape.begin(), button_shape.end(),
                                           []( const cpVect &a, const cpVect &b )
                                           {
                                               return a.y < b.y;
                                           } );


        if (match->butt_beware()) {

            if (is_win >= 0 && min_point->y > best_decision_height && state.is_touch_map == true) {
                best_decision_commands = commands;
                best_decision_height = min_point->y;
            }

            return;
        }

        this->is_win = is_win;

        if (is_win == 1 && tick < min_tick) {
            min_tick = tick;
            best_decision_commands = commands;
            best_decision_height = min_point->y;
        } else if (is_win == 0 && min_point->y > best_decision_height) {
            best_decision_commands = commands;
            best_decision_height = min_point->y;
        } else if (is_win == -1) {
            best_decision_commands = commands;
            best_decision_height = min_point->y;
            this->is_win = 0;
        }
    }

    const list<short> get_commands() {
        return best_decision_commands;
    }

    void set_commands(list<short> commands) {
        best_decision_commands = commands;
    }
};

#endif //MADCARS_EVALUATEFUNC_H
