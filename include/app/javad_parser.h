// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <cstdint>
#include <vector>


namespace app
{
	// Поиск сообщения.
	class StdMessageParser
	{
	private:
		static constexpr const char* _hex = "0123456789ABCDEF";

		static uint8_t _check_sum(const uint8_t* buf, int count)
		{
			uint8_t res = 0;
			while (count--)
				res = ((res << 2) | (res >> 6)) ^ *buf++;
			return ((res << 2) | (res >> 6));
		}

		const int _size;  // Размер данных.
		void* const _msg; // Данные.
		uint8_t _sig[5];  // Заголовок для сообщения.
		bool* _ok_ptr;    // Данные найдены.

		// Проверка остальной части буфера, если совпал первый символ сообщения.
		bool _process(const uint8_t* buf)
		{
			for (int i = 1; i < 5; ++i)
			{
				if (buf[i] != _sig[i])
					return false;
			}
			const int count = _size + 4;
			if (_check_sum(buf, count) != buf[count])
				return false;
			std::memcpy(_msg, buf + 5, _size - 1);
			return true;
		}

	public:
		// msg - указатель на структуру данных.
		template <typename T>
		StdMessageParser(T* msg, const char* name, int size):
			_size(size), _msg(msg)
		{
			_sig[0] = name[0];
			_sig[1] = name[1];
			_sig[2] = _hex[(_size >> 8) & 0xF];
			_sig[3] = _hex[(_size >> 4) & 0xF];
			_sig[4] = _hex[(_size >> 0) & 0xF];
			_ok_ptr = &msg->ok;
		}

		// Поиск сообщения в массиве данных.
		// Вернёт индекс начала следующего сообщения.
		int find(const uint8_t* buf, int len, int i = 0)
		{
			// 5 (заголовок).
			const int i_end = len - 5 - _size;
			for (; i < i_end; ++i)
			{
				if (buf[i] != _sig[0])
					continue;
				if (!_process(buf + i))
					continue;
				*_ok_ptr = true;
				return i + 5 + _size;
			}
			*_ok_ptr = false;
			return -1;
		}

		// Поиск последнего сообщения в массиве данных.
		bool find_end(const uint8_t* buf, int len)
		{
			// 5 (заголовок).
			for (int i = len - 5 - _size; i >= 0; --i)
			{
				if (buf[i] != _sig[0])
					continue;
				if (!_process(buf + i))
					continue;
				*_ok_ptr = true;
				return true;
			}
			*_ok_ptr = false;
			return false;
		}

		void reset()
		{
			*_ok_ptr = false;
		}
	};

	class JavadParser
	{
	public:
		// RT
		struct msg_RT
		{
			uint32_t tod;     // Tr modulo 1 day (86400000 ms) [ms]
			bool ok = false;
		};

		// NT
		struct msg_NT
		{
			uint32_t tod;     // time of day [ms]
			uint16_t dn;      // GLONASS day number (modulo 4 years starting from 1996) []
			uint8_t cycle;    // number of 4-years cycle
			bool ok = false;
		};

		// PV
		struct msg_PV
		{
			double x, y, z;   // Cartesian coordinates [m]
			float p_sigma;    // Position 3D RMS [m]
			float vx, vy, vz; // Cartesian velocities [m/s]
			float v_sigma;    // Velocity 3D RMS [m/s]
			uint8_t sol_type; // Solution type
			bool ok = false;
		};

		// PG
		struct msg_PG
		{
			double lat;       // Latitude [rad]
			double lon;       // Longitude [rad]
			double alt;       // Ellipsoidal height [m]
			float p_sigma;    // Position 3D RMS [m]
			uint8_t sol_type; // Solution type
			bool ok = false;
		};

		// VG
		struct msg_VG
		{
			float lat;        // Northing velocity [m/s]
			float lon;        // Easting velocity [m/s]
			float alt;        // Height velocity [m/s]
			float v_sigma;    // Velocity 3D RMS [m/s]
			uint8_t sol_type; // Solution type
			bool ok = false;
		};

	private:
		msg_RT _msg_RT = {};
		msg_NT _msg_NT = {};
		msg_PV _msg_PV = {};
		msg_PG _msg_PG = {};
		msg_VG _msg_VG = {};
		std::vector<StdMessageParser> _msg_list;

	public:
		JavadParser()
		{
			// Какие сообщения надо искать.
			_msg_list.emplace_back(&_msg_RT, "~~", 5);
			// _msg_list.emplace_back(&_msg_NT, "NT", 8);
			// _msg_list.emplace_back(&_msg_PV, "PV", 46);
			_msg_list.emplace_back(&_msg_PG, "PG", 30);
			_msg_list.emplace_back(&_msg_VG, "VG", 18);
		}

		void update(const uint8_t* data, int len)
		{
			for (auto& msg : _msg_list)
				msg.find_end(data, len);
		}

		void reset()
		{
			for (auto& msg : _msg_list)
				msg.reset();
		}

		const auto& get_RT() const
		{
			return _msg_RT;
		}

		const auto& get_NT() const
		{
			return _msg_NT;
		}

		const auto& get_PV() const
		{
			return _msg_PV;
		}

		const auto& get_PG() const
		{
			return _msg_PG;
		}

		const auto& get_VG() const
		{
			return _msg_VG;
		}
	};
}