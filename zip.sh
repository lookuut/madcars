#!/bin/bash
zip -r madcars.zip __build__.sh solution/ chipmunk_src/  -i \*.h \*.cpp \*.c \*.sh --exclude=madcars/cmake-build-debug/*
zip -ur madcars.zip Makefile

