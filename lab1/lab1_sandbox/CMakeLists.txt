cmake_minimum_required(VERSION 3.29)
project(lab1 C)

set(CMAKE_C_STANDARD 17)

add_executable(lab1 main.c
        state_machine.c
        state_machine.h)
