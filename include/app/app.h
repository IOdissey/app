// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <signal.h>
#include <iostream>
#include "time.h"


namespace app
{
	namespace _
	{
		volatile bool is_run = false;
		bool is_beg = true;

		void handler(int)
		{
			is_run = false;
		}
	}

	bool run(uint32_t us = 0)
	{
		if (_::is_beg)
		{
			signal(SIGINT, _::handler);
			_::is_beg = false;
			_::is_run = true;
		}
		if (us > 0)
			time::sleep_us(us);
		return _::is_run;
	}
}