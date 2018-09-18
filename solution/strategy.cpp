
//#define RELEASE 1
//#define LOCAL_RUNNER 1
#define EMULATION 1
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

    while (true) {

        string input_string = input.get_tick();
        if (input_string == "") {
            break;
        }

        auto input_json = nlohmann::json::parse(input_string);

        if (input_json["type"].get<string>() == "new_match") {

            string tick_string = input.get_tick();

            auto cars_config = nlohmann::json::parse(tick_string);

            game.next_match(input_json, cars_config);

            for (int i = 0; i <= Game::WAIT_STEP_SIZE; i++) {
                game.add_future_step(Game::default_step);
            }

            Game::send_command(Game::default_step, "First step");
        } else if (input_json["type"].get<string>() == "tick") {
            game.tick(input_json);
        }
    }

    return 0;
}