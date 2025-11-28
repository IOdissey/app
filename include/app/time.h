// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <stdint.h>
#include <string.h>
#include <time.h>


namespace app
{
	namespace time
	{
		uint64_t now()
		{
			timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);
			return static_cast<uint64_t>(1000000000ULL) * now.tv_sec + now.tv_nsec;
		}

		namespace _
		{
			uint64_t ns0 = now();
		}

		// Время с начала запуска программы (нс).
		uint64_t ns()
		{
			return now() - _::ns0;
		}

		// Время с начала запуска программы (мкс).
		uint64_t us()
		{
			return (ns() + 500UL) / 1000UL;
		}

		// Время с начала запуска программы (мс).
		uint32_t ms()
		{
			return static_cast<uint32_t>((ns() + 500000UL) / 1000000UL);
		}

		void sleep_ns(uint32_t ns, uint32_t sec = 0)
		{
			timespec deadline;
			clock_gettime(CLOCK_MONOTONIC, &deadline);
			deadline.tv_nsec += ns;
			deadline.tv_sec += sec;
			if(deadline.tv_nsec >= 1000000000L)
			{
				deadline.tv_nsec -= 1000000000L;
				deadline.tv_sec++;
			}
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
		}

		void sleep_us(uint32_t us)
		{
			sleep_ns(us * 1000U);
		}

		void sleep_ms(uint32_t ms)
		{
			if (ms > 1000U)
				sleep_ns((ms % 1000U) * 1000000U, ms / 1000U);
			else
				sleep_ns(ms * 1000000U);
		}
	}
}