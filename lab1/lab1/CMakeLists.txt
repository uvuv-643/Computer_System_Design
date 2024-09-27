cmake_minimum_required(VERSION 3.29)
project(lab1 C)

set(CMAKE_C_STANDARD 17)

add_executable(lab1 main.c
        states.h
        constants.h
        drivers/read_driver.h
        drivers/read_driver.c
        drivers/write_driver.c
        drivers/write_driver.h)
