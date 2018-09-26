
//#define RELEASE 1
//#define LOCAL_RUNNER 1
//#define EMULATION 1
#define FILE_STREAM 1

#include <bits/stdc++.h>
#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>
#include "../../nlohmann/json.hpp"

#include "mechanic/Game.h"
#include <unistd.h>


#include "utils/ShapeCounter.h"
#include <chipmunk/chipmunk.h>
#include "mechanic/InputSource.h"

#include "utils/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

using namespace std;

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
    long micros_sum = 0;

    short command;
    string message = "";
    string input_string = "";
    try {
        while (true) {
            system_clock::time_point start = system_clock::now();
            input_string = input.get_tick();

            if (input_string == "") {
                break;
            }

            auto input_json = nlohmann::json::parse(input_string);

            if (input_json["type"].get<string>() == "new_match") {
                string tick_string = input.get_tick();

                auto cars_config = nlohmann::json::parse(tick_string);
                game.next_match(input_json, cars_config);


                if (game.get_current_match()->butt_beware()) {
                    short balance_command = (cars_config["params"]["my_car"][2] == 1 ? 1 : 0);
                    int wait_tick = game.get_current_match()->butt_beware_wait_tick();

                    for (int i = 0; i < wait_tick; i++) {
                        game.add_future_step(balance_command);
                    }
                } else {
                    game.forecast(-1);
                }


                command = game.get_first_step();
                message = "First step";
            } else if (input_json["type"].get<string>() == "tick") {
                command = game.tick(input_json);
            }

            int micros = 0;

            message = "Round " + std::to_string(game.round) + " tick " + std::to_string(game.match_tick) +
                      " tick micros sum " + std::to_string((double)game.micro_sum / 1000000.0) + " sum of micros " +
                      std::to_string((double)micros_sum / 1000.0) + " micros " + std::to_string(micros_sum);
            game.send_command(command, message);

            system_clock::time_point end = system_clock::now();
            micros = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            micros_sum += micros;
#ifdef LOCAL_RUNNER
            LOG(INFO) << message;
#endif
            }
    } catch (const std::exception &e) {
        game.send_command(Constants::CAR_STOP_COMMAND, e.what());
        throw;
    }

    return 0;
}

