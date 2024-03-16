#
# Copyright (c) 2024 Torben Kalkhof
# Copyright (c) 2024 Embedded Systems and Applications Group, TU Darmstadt
#
# SPDX-License-Identifier: MIT
#

cmake_minimum_required(VERSION 3.20)

project(arraysum-complex)

find_package(HPX REQUIRED)
find_package(Tapasco REQUIRED)
find_package(TapascoHPX REQUIRED)

add_executable(arraysum-complex main.cpp)
target_link_libraries(arraysum-complex HPX::hpx HPX::wrap_main tapasco TapascoHPX)
