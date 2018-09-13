#!/bin/bash
export COMPILED_FILE_PATH="/home/lookuut/Projects/C++/madcars/a.out"
export SOLUTION_CODE_PATH="/home/lookuut/Projects/C++/madcars"
export SOLUTION_LIBRARY_PATH=$SOLUTION_CODE_PATH/chipmunk_src
make -C $SOLUTION_LIBRARY_PATH -f /home/lookuut/Projects/miniaicups/madcars/dockers/cpp17chipmunk/LibMakefile && make -C $SOLUTION_CODE_PATH -f /home/lookuut/Projects/miniaicups/madcars/dockers/cpp17chipmunk/SolutionMakefile
