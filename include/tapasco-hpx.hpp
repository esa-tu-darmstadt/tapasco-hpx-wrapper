/*
 * Copyright (c) 2023-2024 Torben Kalkhof
 * Copyright (c) 2023-2024 Embedded Systems and Applications Group, TU Darmstadt
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TAPASCO_HPX_WRAPPER_TAPASCO_HPX_HPP
#define TAPASCO_HPX_WRAPPER_TAPASCO_HPX_HPP

#include <tapasco.hpp>
#include <hpx/thread.hpp>

using namespace tapasco;

template <typename R, typename... Targs>
void tapasco_hpx_job(Tapasco const &tapasco, PEId peid, RetVal<R> &ret, Targs... args) {
	JobFuture job;
	while (true) {
		if (tapasco.try_launch(job, peid, ret, args...)) {
			hpx::this_thread::suspend();
			continue;
		}
		break;
	}

	while (true) {
		if (job(false)) {
			hpx::this_thread::suspend();
			continue;
		}
		break;
	}
}

template <typename... Targs>
void tapasco_hpx_job(Tapasco &tapasco, PEId peid, Targs... args) {
	JobFuture job;
	while (true) {
		if (tapasco.try_launch(job, peid, args...)) {
			hpx::this_thread::suspend();
			continue;
		}
		break;
	}

	while (true) {
		if (job(false)) {
			hpx::this_thread::suspend();
			continue;
		}
		break;
	}
}

#endif //TAPASCO_HPX_WRAPPER_TAPASCO_HPX_HPP
