#
# Copyright (c) 2023-2024 Torben Kalkhof
# Copyright (c) 2023-2024 Embedded Systems and Applications Group, TU Darmstadt
#
# SPDX-License-Identifier: MIT
#

project("TapascoHPX")
add_library(TapascoHPX INTERFACE)
target_include_directories(TapascoHPX INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")