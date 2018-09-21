cmake_minimum_required(VERSION 3.12)
project(madcars)

set(CMAKE_C_COMPILER "cc")
set(CMAKE_CXX_COMPILER "g++-7")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-std=c++17 -O3 -m64 -pipe -w -pthread")
set(CMAKE_C_FLAGS "-std=gnu99 -ffast-math -DCHIPMUNK_FFI -g -DCP_USE_CGPOINTS=0 -DNDEBUG -fPIC -O3 -m64")

#remove after debug
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

include_directories(chipmunk_src/include)
include_directories(chipmunk_src/include/chipmunk)
include_directories(chipmunk_src/src)

add_executable(madcars
        chipmunk_src/include/chipmunk/chipmunk.h
        chipmunk_src/include/chipmunk/chipmunk_ffi.h
        chipmunk_src/include/chipmunk/chipmunk_private.h
        chipmunk_src/include/chipmunk/chipmunk_structs.h
        chipmunk_src/include/chipmunk/chipmunk_types.h
        chipmunk_src/include/chipmunk/chipmunk_unsafe.h
        chipmunk_src/include/chipmunk/cpArbiter.h
        chipmunk_src/include/chipmunk/cpBB.h
        chipmunk_src/include/chipmunk/cpBody.h
        chipmunk_src/include/chipmunk/cpConstraint.h
        chipmunk_src/include/chipmunk/cpDampedRotarySpring.h
        chipmunk_src/include/chipmunk/cpDampedSpring.h
        chipmunk_src/include/chipmunk/cpGearJoint.h
        chipmunk_src/include/chipmunk/cpGrooveJoint.h
        chipmunk_src/include/chipmunk/cpHastySpace.h
        chipmunk_src/include/chipmunk/cpMarch.h
        chipmunk_src/include/chipmunk/cpPinJoint.h
        chipmunk_src/include/chipmunk/cpPivotJoint.h
        chipmunk_src/include/chipmunk/cpPolyline.h
        chipmunk_src/include/chipmunk/cpPolyShape.h
        chipmunk_src/include/chipmunk/cpRatchetJoint.h
        chipmunk_src/include/chipmunk/cpRobust.h
        chipmunk_src/include/chipmunk/cpRotaryLimitJoint.h
        chipmunk_src/include/chipmunk/cpShape.h
        chipmunk_src/include/chipmunk/cpSimpleMotor.h
        chipmunk_src/include/chipmunk/cpSlideJoint.h
        chipmunk_src/include/chipmunk/cpSpace.h
        chipmunk_src/include/chipmunk/cpSpatialIndex.h
        chipmunk_src/include/chipmunk/cpTransform.h
        chipmunk_src/include/chipmunk/cpVect.h
        chipmunk_src/include/madcars_allocator.h
        chipmunk_src/src/madcars_allocator.c
        chipmunk_src/src/chipmunk.c
        chipmunk_src/src/cpArbiter.c
        chipmunk_src/src/cpArray.c
        chipmunk_src/src/cpBBTree.c
        chipmunk_src/src/cpBody.c
        chipmunk_src/src/cpCollision.c
        chipmunk_src/src/cpConstraint.c
        chipmunk_src/src/cpDampedRotarySpring.c
        chipmunk_src/src/cpDampedSpring.c
        chipmunk_src/src/cpGearJoint.c
        chipmunk_src/src/cpGrooveJoint.c
        chipmunk_src/src/cpHashSet.c
        chipmunk_src/src/cpHastySpace.c
        chipmunk_src/src/cpMarch.c
        chipmunk_src/src/cpPinJoint.c
        chipmunk_src/src/cpPivotJoint.c
        chipmunk_src/src/cpPolyline.c
        chipmunk_src/src/cpPolyShape.c
        chipmunk_src/src/cpRatchetJoint.c
        chipmunk_src/src/cpRobust.c
        chipmunk_src/src/cpRotaryLimitJoint.c
        chipmunk_src/src/cpShape.c
        chipmunk_src/src/cpSimpleMotor.c
        chipmunk_src/src/cpSlideJoint.c
        chipmunk_src/src/cpSpace.c
        chipmunk_src/src/cpSpaceComponent.c
        chipmunk_src/src/cpSpaceDebug.c
        chipmunk_src/src/cpSpaceHash.c
        chipmunk_src/src/cpSpaceQuery.c
        chipmunk_src/src/cpSpaceStep.c
        chipmunk_src/src/cpSpatialIndex.c
        chipmunk_src/src/cpSweep1D.c
        chipmunk_src/src/prime.h
        solution/strategy.cpp solution/mechanic/Game.h solution/mechanic/Match.h solution/models/Deadline.cpp solution/models/Deadline.h solution/models/Map.h solution/models/Car.cpp solution/models/Car.h solution/mechanic/Player.h solution/models/Constans.h solution/utils/easylogging++.cpp solution/utils/easylogging++.h solution/utils/ShapeCounter.h solution/mechanic/BaseSimulation.cpp solution/mechanic/BaseSimulation.h solution/mechanic/CarState.cpp solution/mechanic/CarState.h solution/mechanic/Player.cpp solution/mechanic/SimVariance.cpp solution/mechanic/SimVariance.h solution/mechanic/InputSource.h solution/mechanic/FutureSimulation.h solution/mechanic/FutureSimulation.cpp)
