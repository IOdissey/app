// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <cstdint>
#include "time.h"


namespace app
{
	class Rate
	{
	private:
		uint64_t _period = 0; // Период срабатывания.
		uint64_t _point = 0;  // Время следующего срабатывания.

	public:
		void ns(uint32_t period)
		{
			_period = period;
			reset();
		}

		void us(uint32_t period)
		{
			_period = period * 1000UL;
			reset();
		}

		void ms(uint32_t period)
		{
			_period = period * 1000000UL;
			reset();
		}

		bool ok()
		{
			uint64_t now = time::now();
			if (now < _point)
				return false;
			_point += _period;
			if (now > _point)
				_point = now + _period;
			return true;
		}

		void reset()
		{
			_point = time::now() + _period;
		}

		// Возвращает количество оставшихся миллисекунд до срабатывания.
		uint32_t ms() const
		{
			uint64_t now = time::now();
			if (now < _point)
				return static_cast<uint32_t>((_point - now + 500UL) / 1000000UL);
			return 0U;
		}

		// Ожидание следующего срабатывания.
		void wait()
		{
			uint64_t now = time::now();
			while (now < _point)
			{
				time::sleep_ns(static_cast<uint32_t>(_point - now));
				now = time::now();
			}
			_point += _period;
			if (now > _point)
				_point = now + _period;
		}
	};
}