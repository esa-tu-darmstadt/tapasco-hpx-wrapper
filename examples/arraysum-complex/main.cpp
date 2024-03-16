/*
 * Copyright (c) 2024 Torben Kalkhof
 * Copyright (c) 2024 Embedded Systems and Applications Group, TU Darmstadt
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <hpx/init.hpp>
#include <hpx/future.hpp>
#include <hpx/algorithm.hpp>

#include <tapasco-hpx.hpp>

#define TASK_SZ 256
#define TOTAL_SZ 32768
#define NUM_TASKS (TOTAL_SZ / TASK_SZ)
#define PE_NAME "esa.cs.tu-darmstadt.de:hls:arraysum:1.0"

// we could calculate this easier...
int do_ref_calculation() {
        int sum = 0;
        for (size_t i = 0; i < TOTAL_SZ; ++i) {
                sum += i;
        }
        return sum;
}

tapasco::DeviceAddress copy_array(tapasco::Tapasco &tapasco, int *data) {
        // allocate memory and copy data
        tapasco::DeviceAddress dev_addr = 0;
        if (tapasco.alloc(dev_addr, TASK_SZ * sizeof(int))) {
                std::cout << "Failed to allocate memory" << std::endl;
                hpx::terminate();
        }
        if (tapasco.copy_to((uint8_t *)data, dev_addr, TASK_SZ * sizeof(int))) {
                std::cout << "Data transfer to device failed" << std::endl;
                hpx::terminate();
        }
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

int sumup_results(std::vector<int> results) {
        int sum = 0;
        for (int &r : results) {
                sum += r;
        }
        return sum;
}

hpx::future<int> do_hw_calculation(tapasco::Tapasco &tapasco, tapasco::PEId pe_type, std::array<int, TOTAL_SZ> &buf) {
        // use IO pool executor for copy tasks
        hpx::parallel::execution::io_pool_executor io_executor;
        std::vector<hpx::future<int>> futures;
        for (size_t i = 0; i < NUM_TASKS; ++i) {
                // copy task to device memory
                int *sub = buf.data() + i * TASK_SZ;
                hpx::future<tapasco::DeviceAddress> fut = hpx::async(io_executor, &copy_array, std::ref(tapasco), sub);

                // add calculation as continuation
                futures.push_back(fut.then([&tapasco, pe_type](hpx::future<tapasco::DeviceAddress> &&f) {
                        return run_arraysum(tapasco, pe_type, f.get());
                }));
        }

        // sum up results
        return hpx::dataflow(hpx::unwrapping_all(sumup_results), hpx::when_all(futures));
}

bool check_result(int reference, int result) {
        return reference == result;
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

        // launch reference computation asynchronously
        hpx::shared_future<int> reference = hpx::async(do_ref_calculation);

        // prepare buffer
        std::array<int, TOTAL_SZ> buf;
        hpx::experimental::for_loop(hpx::execution::par, 0, TOTAL_SZ, [&](int i) {
                buf[i] = i;
        });

        // check whether number of runs would exceed memory capacity on device
        if (vm["runs"].as<std::size_t>() * TOTAL_SZ * sizeof(int) > 4UL << 30) {
                std::cout << "Memory capacity exceeded0" << std::endl;
                hpx::finalize(1);
        }

        // launch tasks on hardware and check results
        std::vector<hpx::future<bool>> futures;
        for (std::size_t i = 0; i < vm["runs"].as<std::size_t>(); ++i) {
                hpx::future<int> result = do_hw_calculation(tapasco, pe_type, buf);
                futures.push_back(hpx::dataflow(hpx::unwrapping(check_result), reference, result));
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