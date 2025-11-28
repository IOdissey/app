// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <limits>
#include <string>
#include <vector>
#include "ais_payload.h"
#include "config.h"
#include "nmea.h"
#include "tcp_client.h"


namespace app
{
	// Получение данных от AIS.
	class AIS
	{
	private:
		bool _debug = false;
		app::TCPClient _client;
		app::NMEA _nmea;
		std::vector<AISPayload> _res;
		std::string _part_data;       // Буфер для фрагментов сообщения.
		int _part_count = -1;         // Порядковый номер следующего фрагмента.
		int _part_seq = -1;           // Идентификатор фрагмента.

	public:
		AIS()
		{
			_nmea.set_beg_char('!');
		}

		bool beg(const app::Config& cfg)
		{
			_debug = cfg.get("debug", _debug);
			return _client.beg(cfg);
		}

		bool update()
		{
			_res.clear();
			if (!_client.update())
				return false;
			const uint8_t* data = _client.data();
			size_t data_size = _client.data_size();
			if (_debug)
				std::cout << std::string((const char*)data, data_size) << std::endl;
			while (_nmea.update(data, data_size))
			{
				// TODO. AIVDO - для теста.
				// if (_nmea.is_msg("AIVDM", 7) || _nmea.is_msg("AIVDO", 7))
				if (_nmea.is_msg("AIVDM", 7))
				{
					// Целое сообщение.
					int total_part = _nmea.get_int(1);
					if (total_part == 1)
					{
						_res.emplace_back(_nmea.get_str(5));
						continue;
					}
					// Фрагмент сообщения.
					int current_part = _nmea.get_int(2);
					if (current_part == 1)
					{
						_part_data = std::string(_nmea.get_str(5));
						_part_count = 2;
						_part_seq = _nmea.get_int(3);
						continue;
					}
					// Ошибка последовательности.
					else if (current_part != _part_count || _nmea.get_int(3) != _part_seq)
					{
						_part_count = -1;
						_part_seq = -1;
						continue;
					}
					_part_data += std::string(_nmea.get_str(5));
					if (current_part != total_part)
					{
						_part_count++;
						continue;
					}
					_res.emplace_back(_part_data);
					_part_count = -1;
					_part_seq = -1;
				}
			}
			return (_res.size() > 0);
		}

		bool ok() const
		{
			return (_res.size() > 0);
		}

		const std::vector<AISPayload>& get_res() const
		{
			return _res;
		}
	};
}