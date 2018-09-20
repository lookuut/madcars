//
// Created by lookuut on 17.09.18.
//

#ifndef MADCARS_INPUTSOURCE_H
#define MADCARS_INPUTSOURCE_H

#include <iostream>
#include <fstream>
#include <list>
#include "../../../nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

enum input_stream_type {emulation, input_stream , file_stream};

class InputSource {

private:
    json test_steps;
    int steps_counter = 0;

    bool logging = false;
    input_stream_type type;
    ofstream log;
    ifstream file_stream;

public:

    InputSource(input_stream_type type, bool logging) {
        this->type = type;
        this->logging = logging;

        if (this->logging) {
            log.open ("mad-cars-input.log");
        }

        if (type == input_stream_type::file_stream) {
            file_stream.open("mad-cars-input.log");
        } else if (type == input_stream_type::emulation) {
            ifstream visio_file("/home/lookuut/Downloads/visio");
            string line;
            getline(visio_file, line);
            visio_file.close();

            auto visio = nlohmann::json::parse(line);
            test_steps = visio["visio_info"];
        }
    }

    ~InputSource() {

        if (this->logging) {
            log.close();
        }

        if (type == input_stream_type::file_stream) {
            file_stream.close();
        }
    }

    inline string get_tick() {
        string input_string = "";
        if (type == input_stream_type::input_stream) {
            getline(cin, input_string);
        } else if (type == input_stream_type::emulation) {
            auto tick = test_steps[steps_counter];

            if (tick["type"].get<string>() == "tick") {
                tick["params"]["my_car"] = tick["params"]["cars"]["2"];
                tick["params"]["enemy_car"] = tick["params"]["cars"]["1"];
            }

            input_string = tick.dump();
            steps_counter++;
        } else if (type == input_stream_type::file_stream) {
            getline (file_stream, input_string);
        }

        if (this->logging) {
            log << input_string << endl;
        }

        return input_string;
    }
};
#endif //MADCARS_INPUTSOURCE_H
