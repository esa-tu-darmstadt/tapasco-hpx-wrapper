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
#define ARRAYINIT_PE "esa.cs.tu-darmstadt.de:hls:arrayinit:1.0"
#define ARRAYUPDATE_PE "esa.cs.tu-darmstadt.de:hls:arrayupdate:1.0"
#define ARRAYSUM_PE "esa.cs.tu-darmstadt.de:hls:arraysum:1.0"

int calculate_reference() {
        int sum = 0;
        for (int i = 0; i < SZ; ++i) {
                sum += i + 42;
        }
        return sum;
}

tapasco::DeviceAddress run_arrayinit(tapasco::Tapasco &tapasco, tapasco::PEId pe_type) {
        // allocate memory on device
        tapasco::DeviceAddress dev_addr;
        if (tapasco.alloc(dev_addr, SZ * sizeof(int))) {
                std::cout << "Failed to allocate memory" << std::endl;
                hpx::terminate();
        }

        // execute task on PE
        tapasco_hpx_job(tapasco, pe_type, dev_addr);

        return dev_addr;
}

tapasco::DeviceAddress run_arrayupdate(tapasco::Tapasco &tapasco, tapasco::PEId pe_type, tapasco::DeviceAddress dev_addr) {
        // execute task on PE
        tapasco_hpx_job(tapasco, pe_type, dev_addr);

        return dev_addr;
}

int run_arraysum(tapasco::Tapasco &tapasco, tapasco::PEId pe_type, tapasco::DeviceAddress dev_addr) {
        // variable for PE return value
        int result = 0;
        tapasco::RetVal<int> retVal(&result);

        // execute task on PE
        tapasco_hpx_job(tapasco, pe_type, retVal, dev_addr);

        tapasco.free(dev_addr);
        return result;
}

int hpx_main(hpx::program_options::variables_map &vm) {

        // get TaPaSCo object
        tapasco::Tapasco tapasco;

        // get IDs of PE types
        tapasco::PEId initpe_type, updatepe_type, sumpe_type;
        try {
                initpe_type = tapasco.get_pe_id(ARRAYINIT_PE);
                updatepe_type = tapasco.get_pe_id(ARRAYUPDATE_PE);
                sumpe_type = tapasco.get_pe_id(ARRAYSUM_PE);
        } catch (tapasco_error &e) {
                std::cout << "Need at least one PE of each type to run." << std::endl;
                return hpx::finalize(1);
        }

        // create tasks for all runs
        std::vector<hpx::future<int>> futures;
        for (std::size_t i = 0; i < vm["runs"].as<std::size_t>(); ++i) {
                hpx::future<tapasco::DeviceAddress> f = hpx::async(run_arrayinit, std::ref(tapasco), initpe_type);
                futures.push_back(f.then([&tapasco, updatepe_type](auto &&addr) {
                        return run_arrayupdate(tapasco, updatepe_type, addr.get());
                }).then([&tapasco, sumpe_type](auto &&addr) {
                        return run_arraysum(tapasco, sumpe_type, addr.get());
                }));
        }

        // check results
        int reference = calculate_reference();
        std::vector<int> results = hpx::unwrap(futures);
        for (std::size_t i = 0; i < results.size(); ++i) {
                if (results[i] == reference) {
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