cmake_minimum_required(VERSION 3.20)
project(PacmanRaylib CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(PacmanRaylib
    HelloWorld/Main.cpp
    HelloWorld/Game.cpp
    HelloWorld/Pacman.cpp
    HelloWorld/Ghost.cpp
    HelloWorld/RaylibPlayMain.cpp
    HelloWorld/FSM/GhostStateMachine.cpp
    HelloWorld/FSM/GhostStates.cpp
)

target_include_directories(PacmanRaylib PUBLIC HelloWorld HelloWorld/FSM)

target_link_libraries(PacmanRaylib PRIVATE raylib)