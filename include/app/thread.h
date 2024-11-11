// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <thread>
#include "app.h"
#include "time.h"


namespace app
{
	class Thread
	{
	private:
		volatile bool _thread_active = false; // Флаг активности задачи.
		volatile bool _thread_stop = false;   // Команда того, что процесс должен быть остановлен.

		virtual void _thread_run()
		{
		}

	public:
		void thread_run()
		{
			if (_thread_active)
				return;
			_thread_stop = false;
			_thread_active = true;
			std::thread thread([this]()
			{
				while (!_thread_stop)
				{
					app::time::sleep_us(100);
					_thread_run();
				}
				_thread_active = false;
			});
			thread.detach();
		}

		void thread_end()
		{
			_thread_stop = true;
			while (_thread_active)
				app::time::sleep_ms(1);
		}
	};
}