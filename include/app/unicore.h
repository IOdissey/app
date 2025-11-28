// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

// AGRICB
// char    gnss[4];       // 2  H
// uint8_t length;        // 3  H + 4   length = fixed value 232 bytes in 0XE8
// uint8_t Year;          // 4  H + 5   UTC - year, for example: 2016 : 16; 2116: 116
// uint8_t Month;         // 5  H + 6   UTC - Month
// uint8_t Day;           // 6  H + 7   UTC - Day
// uint8_t Hour;          // 7  H + 8   UTC - Hour
// uint8_t Minute;        // 8  H + 8   UTC - Minute
// uint8_t Second;        // 9  H + 10  UTC - Second
// uint8_t Postype;       // 10 H + 11  Rover position status: 0: Invalid solution; 1: Single point solution; 2: Pseudorange differential solution; 4: Fixed solution; 5: Float solution; 7: Input a fixed position (only supported by specific versions)
// uint8_t HeadStatus;    // 11 H + 12  Heading solution status of master and slave antennas: 0: Invalid solution; 4: Fixed solution; 5: Float solution
// uint8_t NumGPSSta;     // 12 H + 13  Number of GPS satellites used in the solution
// uint8_t NumBDSSta;     // 13 H + 14  Number of BDU satellites used in the solution
// uint8_t NumGLOSta;     // 14 H + 15  Number of GLO satellites used in the solution
// float   Baseline_N;    // 15 H + 16  Baseline vector from the base station to the rover station, northern component
// float   Baseline_E;    // 16 H + 20  Baseline vector from the base station to the rover station, eastern component
// float   Baseline_U;    // 17 H + 24  Baseline vector from the base station to the rover station, eastern component
// float   Baseline_NStd; // 18 H + 28  Baseline vector from the base station to the rover station, northern component standard deviation
// float   Baseline_EStd; // 19 H + 32  Baseline vector from the base station to the rover station, eastern component standard deviation
// float   Baseline_UStd; // 20 H + 36  Baseline vector from the base station to the rover station, eastern component standard deviation
// float   Heading;       // 21 H + 40
// float   Pitch;         // 22 H + 44
// float   Roll;          // 23 H + 48
// float   Speed;         // 24 H + 52
// float   Velocity_N;    // 25 H + 56
// float   Velocity_E;    // 26 H + 60
// float   Velocity_U;    // 27 H + 64
// float   Velocity_NStd; // 28 H + 68
// float   Velocity_EStd; // 29 H + 72
// float   Velocity_UStd; // 30 H + 76
// double  Lat;           // 31 H + 80
// double  Lon;           // 32 H + 88
// double  Alt;           // 33 H + 96
// double  X_ecef;        // 34 H + 104 X axis of the ECEF coordinate system
// double  Y_ecef;        // 35 H + 112 Y axis of the ECEF coordinate system
// double  Z_ecef;        // 36 H + 120 Z axis of the ECEF coordinate system
// float   LatStd;        // 37 H + 128
// float   LonStd;        // 38 H + 132
// float   AltStd;        // 39 H + 136
// float   X_ecefStd;     // 40 H + 140 X axis of the ECEF coordinate system standard deviation
// float   Y_ecefStd;     // 41 H + 144 Y axis of the ECEF coordinate system standard deviation
// float   Z_ecefStd;     // 42 H + 148 Z axis of the ECEF coordinate system standard deviation
// double  BLat;          // 43 H + 152 Latitude of the base station: -90~90 degrees
// double  BLon;          // 44 H + 160 Longitude of the base station: -180~180 degrees
// double  BAlt;          // 45 H + 168 Height of the base station
// double  SLat;          // 46 H + 176 Latitude of the slave antenna: -90~90 degrees
// double  SLon;          // 47 H + 184 Longitude of the slave antenna: -180~180 degrees
// double  SAlt;          // 48 H + 192 Height of the slave antenna:
// int32_t tow;           // 49 H + 200 Milliseconds of GPS week
// float   Diffage;       // 50 H + 204
// float   Speed_Heading; // 51 H + 208
// float   Undulation;    // 52 H + 212
// float   fReserved[2];  // 53-54 H + 216
// uint8_t NumGALSta;     // 55 H + 224 Number of GAL satellites used in the solution
// uint8_t Speed_Type;    // 56 H + 225 0: speed solution status valid; 1: speed solution status invalid
// uint8_t cReserved[2];  // 57-58 H + 226
// uint8_t cs[4];         // 59 H + 228


#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include "config.h"
#include "serial.h"
#include "thread.h"


namespace app
{
	// Данные с ГНСС.
	class Unicore : public Thread
	{
	public:
		struct agricb_struct
		{
			int32_t tow = 0;
			int status = 0;
			double lat = 0.0;
			double lon = 0.0;
			double alt = 0.0;
			float vn = 0.0f;
			float ve = 0.0f;
			float vu = 0.0f;
			int head_status = 0;
			float heading = 0.0f;
			float pitch = 0.0f;
			float roll = 0.0f;
			int sat = 0;
			double vel_2d_std = 0.0;
			double pos_3d_std = 0.0;
		};

	private:
		enum class state_enum
		{
			BEG,
			TYPE,
			CRC,
			OK
		};

		state_enum _state = state_enum::BEG;
		Serial _serial;
		std::vector<uint8_t> _buf;
		size_t _buf_size = 0;                // Размер буфера с данными.
		size_t _msg_idx = 0;                 // Указатель на начало сообщения.
		uint16_t _msg_type = 0;
		size_t _msg_size = 0;

		agricb_struct _agricb;
		volatile bool _agricb_ok = false;
		agricb_struct _agricb_res;
		bool _agricb_res_ok = false;

		template <typename T>
		T _get_data(int i) const
		{
			return *((T*)&_buf[_msg_idx + i]);
		}

		// Поиск начала сообщения.
		void _parse_beg()
		{
			if (_buf_size < _msg_idx + 3)
				return;
			const size_t size = _buf_size - 2;
			for (; _msg_idx < size; ++_msg_idx)
			{
				if (_buf[_msg_idx] == 0xAA && _buf[_msg_idx + 1] == 0x44 && _buf[_msg_idx + 2] == 0xB5)
				{
					_state = state_enum::TYPE;
					return;
				}
			}
			_buf_size = 0;
			_msg_idx = 0;
		}

		// Тип сообщения и размер.
		void _parse_type()
		{
			// Размер заголовка: header(24).
			if (_buf_size < _msg_idx + 24)
				return;
			_msg_type = _get_data<uint16_t>(4);
			_msg_size = _get_data<uint16_t>(6);
			// AGRICB
			if (_msg_type == 11276 && _msg_size == 228)
			{
				// Полный размер сообщения.
				_msg_size += 28; // header(24) + crc(4)
				_state = state_enum::CRC;
			}
			else
			{
				_state = state_enum::BEG;
				_msg_idx += 3;
			}
		}

		// Проверка контрольной суммы.
		void _parse_crc()
		{
			// Размер всего сообщения.
			if (_buf_size < _msg_idx + _msg_size)
				return;
			// Расчёт контрольной суммы.
			const int _data_len = static_cast<int>(_msg_size) - 4;
			uint32_t crc = 0;
			const uint8_t* buf = &_buf[_msg_idx];
			for (int i = 0; i < _data_len; ++i)
			{
				crc ^= buf[i];
				for (int j = 0; j < 8; ++j)
				{
					if (crc & 1)
						crc = (crc >> 1) ^ 0xEDB88320u; // CRC32 polynomial
					else
						crc >>= 1;
				}
			}
			// Контрольная сумма не совпала.
			if (_get_data<uint32_t>(_data_len) == crc)
				_state = state_enum::OK;
			else
			{
				_state = state_enum::BEG;
				_msg_idx += 3;
			}
		}

		// Сообщение готово.
		void _parse_ok()
		{
			// AGRICB
			if (_msg_type == 11276)
			{
				// Данные ещё не вычитаны.
				if (_agricb_ok)
					return;
				_agricb.status = _get_data<uint8_t>(24 + 11);
				_agricb.vn = _get_data<float>(24 + 56);
				_agricb.ve = _get_data<float>(24 + 60);
				_agricb.vu = _get_data<float>(24 + 64);
				_agricb.lat = _get_data<double>(24 + 80);
				_agricb.lon = _get_data<double>(24 + 88);
				// _agricb.lat_std = _get_data<float>(24 + 128);
				// _agricb.lon_std = _get_data<float>(24 + 132);
				_agricb.alt = _get_data<double>(24 + 96);
				_agricb.tow = _get_data<int32_t>(24 + 200);
				_agricb.sat = _get_data<uint8_t>(24 + 13) + _get_data<uint8_t>(24 + 14) + _get_data<uint8_t>(24 + 15) + _get_data<uint8_t>(24 + 224);
				_agricb.head_status = _get_data<uint8_t>(24 + 12);
				_agricb.heading = _get_data<float>(24 + 40);
				_agricb.pitch = _get_data<float>(24 + 44);
				_agricb.roll = _get_data<float>(24 + 48);
				//
				double vx_std = _get_data<float>(24 + 68);
				double vy_std = _get_data<float>(24 + 72);
				_agricb.vel_2d_std = std::sqrt(vx_std * vx_std + vy_std * vy_std);
				//
				double px_std = _get_data<float>(24 + 140);
				double py_std = _get_data<float>(24 + 144);
				double pz_std = _get_data<float>(24 + 148);
				_agricb.pos_3d_std = std::sqrt(px_std * px_std + py_std * py_std + pz_std * pz_std);
				//
				_agricb_ok = true;
			}
			_buf_size -= _msg_idx + _msg_size;
			if (_buf_size > 0)
				std::memmove(&_buf[0], &_buf[_msg_idx + _msg_size], _buf_size);
			_msg_idx = 0;
			_state = state_enum::BEG;
		}

		// Чтение данных из серийного порта в отдельном потоке.
		void _thread_run()
		{
			// Если буфер заполнен, то сброс.
			if (_buf_size == _buf.size())
			{
				_buf_size = 0;
				_state = state_enum::BEG;
				std::cout << "Unicore buffer reset." << std::endl;
			}
			// Чтение порции данных.
			_buf_size += _serial.read_data(&_buf[_buf_size], _buf.size() - _buf_size);
			// Разбор сообщения.
			if (_state == state_enum::BEG)
				_parse_beg();
			if (_state == state_enum::TYPE)
				_parse_type();
			if (_state == state_enum::CRC)
				_parse_crc();
			if (_state == state_enum::OK)
				_parse_ok();
		}

	public:
		~Unicore()
		{
			end();
		}

		bool beg(const app::Config& cfg)
		{
			end();
			std::string port = cfg.get<std::string>("port", "/dev/ttyUSB0");
			int baud = cfg.get<int>("baud", 115200);
			if (!_serial.serial_open(port, baud))
				return false;
			_buf.resize(cfg.get<uint32_t>("buf_size", 1024));
			thread_run();
			return true;
		}

		void end()
		{
			thread_end();
			_serial.serial_close();
		}

		void update()
		{
			_agricb_res_ok = _agricb_ok;
			if (_agricb_ok)
			{
				_agricb_res = _agricb;
				_agricb_ok = false;
			}
		}

		bool is_agricb() const
		{
			return _agricb_res_ok;
		}

		const agricb_struct& get_agricb() const
		{
			return _agricb_res;
		}
	};
}