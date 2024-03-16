/*
 * Copyright (c) 2024 Torben Kalkhof
 * Copyright (c) 2024 Embedded Systems and Applications Group, TU Darmstadt
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <hpx/init.hpp>
#include <hpx/future.hpp>

#include <tapasco-hpx.hpp>

#define SZ 256
#define PE_NAME "esa.cs.tu-darmstadt.de:hls:arraysum:1.0"

bool run_arraysum(tapasco::Tapasco &tapasco, tapasco::PEId pe_type) {
        std::array<int, SZ> buf;
        int ref = 0;

        // initialize array
        for (std::size_t i = 0; i < buf.size(); ++i) {
                buf[i] = i;
                ref += i;
        }

        // mark buffer for data transfer to device memory
        auto ptr = tapasco::makeInOnly(tapasco::makeWrappedPointer(buf.data(), buf.size() * sizeof(int)));

        // variable for PE return value
        int result = 0;
        tapasco::RetVal<int> retVal(&result);

        // execute task on PE
        tapasco_hpx_job(tapasco, pe_type, retVal, ptr);

        // check result
        return (ref == result);
}

int hpx_main(hpx::program_options::variables_map &vm) {

        // get TaPaSCo object
        tapasco::Tapasco tapasco;

        // get ID of PE type
        tapasco::PEId pe_type = 0;
        try {
                pe_type = tapasco.get_pe_id(PE_NAME);
        } catch (tapasco_error &e) {
                std::cout << "Need at least one arraysum PE to run." << std::endl;
                return hpx::finalize(1);
        }

        // create tasks for all runs
        std::vector<hpx::future<bool>> futures;
        for (std::size_t i = 0; i < vm["runs"].as<std::size_t>(); ++i) {
                futures.push_back(hpx::async(run_arraysum, std::ref(tapasco), pe_type));
        }

        // check results
        std::vector<bool> results = hpx::unwrap(futures);
        for (std::size_t i = 0; i < results.size(); ++i) {
                if (results[i]) {
                        std::cout << "RUN " << i << ": OK" << std::endl;
                } else {
                        std::cout << "RUN " << i << ": NOT OK" << std::endl;
                }
        }

        return hpx::finalize();
}

int main(int argc, char **argv) {
        using namespace hpx::program_options;

        options_description desc;
        desc.add_options()
                ("runs", value<std::size_t>()->default_value(25), "number of total runs");

        hpx::init_params init_args;
        init_args.desc_cmdline = desc;

        return hpx::init(argc, argv, init_args);
}