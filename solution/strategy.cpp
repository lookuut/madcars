
//#define RELEASE 1
#define LOCAL_RUNNER 1
//#define EMULATION 1
//#define FILE_STREAM 1
#include <bits/stdc++.h>
#include <iostream>
#include "../../nlohmann/json.hpp"

#include "mechanic/Game.h"

#include "utils/ShapeCounter.h"
#include <chipmunk/chipmunk.h>
#include "mechanic/InputSource.h"

#include "utils/easylogging++.h"
INITIALIZE_EASYLOGGINGPP


using namespace std;

#define PI 3.14159265358979323846264338327950288

short smart_guy(json & state) {
    string input_type = state["type"].get<std::string>();

    auto params = state["params"];
    auto my_car = params["my_car"];
    auto enemy_car = params["enemy_car"];
    auto my_pos = my_car[0];
    auto enemy_pos = enemy_car[0];
    short command = 2;

    // check my and enemy position and go to the enemy

    if(my_pos[0].get<double>() > enemy_pos[0].get<double>()) {
        command = 0;
    } else {
        command = 1;
    }
    // roll over  in air prevention (this feature can lead to death)
    double my_angle = my_car[1].get<double>();
    // normalize angle
    while (my_angle > PI) {
        my_angle -= 2.0 * PI;
    }
    while (my_angle < -PI) {
        my_angle += 2.0 * PI;
    }

    if (my_angle > PI / 4.0) {
        //cerr << "Uhh!" << endl;
        command = 0;
    } else if (my_angle < -PI / 4.0) {
        //cerr << "Ahh!" << endl;
        command = 1;
    }

    return command;
}

int main() {

#ifndef RELEASE
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.set(el::Level::Global,
                    el::ConfigurationType::Filename, "mad-cars.log");
    defaultConf.set(el::Level::Global,
                    el::ConfigurationType::ToFile, "true");
    defaultConf.set(el::Level::Global,
                    el::ConfigurationType::ToStandardOutput, "false");

    el::Loggers::reconfigureLogger("default", defaultConf);
#endif

    Game game = Game();

#ifdef RELEASE
    InputSource input(input_stream_type::input_stream, false);
#endif

#ifdef LOCAL_RUNNER
    InputSource input(input_stream_type::input_stream, true);
#endif

#ifdef EMULATION
    InputSource input(input_stream_type::emulation, false);
#endif

#ifdef FILE_STREAM
    InputSource input(input_stream_type::file_stream, false);
#endif

    string input_string = "";
    while (true) {
        try {

            input_string = input.get_tick();
            if (input_string == "") {
                break;
            }

            auto input_json = nlohmann::json::parse(input_string);

            if (input_json["type"].get<string>() == "new_match") {

                string tick_string = input.get_tick();

                auto cars_config = nlohmann::json::parse(tick_string);

                game.next_match(input_json, cars_config);
                game.forecast(-1);

                game.send_command(game.get_first_step(), "First step");
            } else if (input_json["type"].get<string>() == "tick") {
                game.tick(input_json);
            }

        } catch (const std::exception& e) {
#ifdef LOCAL_RUNNER
            throw ;
#endif
#ifdef FILE_STREAM
            throw ;
#endif

            auto input_json = nlohmann::json::parse(input_string);
            short smart_command = smart_guy(input_json);

            game.send_command(smart_command, "Smart guy action");
            game.run_empty(smart_command);


            while (true) {
                input_string = input.get_tick();
                auto input_json = nlohmann::json::parse(input_string);

                if (input_json["type"].get<string>() == "new_match") {

                    string tick_string = input.get_tick();

                    auto cars_config = nlohmann::json::parse(tick_string);

                    game.next_match(input_json, cars_config);

                    for (int i = 0; i <= Constants::MATCH_START_WAIT_TICKS; i++) {
                        game.add_future_step(Constants::MATCH_START_STEP);
                    }

                    game.send_command(Constants::MATCH_START_STEP, "First step");
                    break;

                } else {
                    short smart_command = smart_guy(input_json);

                    game.send_command(smart_command, "Smart guy action");
                    game.run_empty(smart_command);
                }
            }

        }
    }

    return 0;
}

