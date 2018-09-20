//
// Created by lookuut on 10.09.18.
//

#ifndef MADCARS_CONSTANS_H
#define MADCARS_CONSTANS_H


class Constants {
public:
    static const short CAR_STOP_COMMAND = 2;
    static const  int TICKS_TO_DEADLINE = 600;
    static const  int MATCHES = 600;
    static const  int MAX_EXECUTION_TIME = 120;
    static const  int REQUEST_MAX_TIME = 5;
    static const  int MAX_TICK_COUNT = 20000;
    static const  int SEED = 1;
    static const  int REST_TICKS = 90;
    static const  int MAX_SPACES_COUNT = 1000;
    static constexpr double SPACE_TICK = 0.016;
    static const short STEP_MAX_SIZE = 2;
    static const int default_step = 2;
    static const int MATCH_START_WAIT_TICKS = 10;
    static const short MATCH_START_STEP = CAR_STOP_COMMAND;

};

#endif //MADCARS_CONSTANS_H
