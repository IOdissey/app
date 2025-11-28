// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>


namespace app
{
	// Разбор NMEA сообщения.
	class NMEA
	{
	private:
		enum class state_enum
		{
			NONE,
			BEG,
			CRC1,
			CRC2,
			OK
		};

		static const char* _hex;               // Для преобразования полубайта в символ.
		static const uint16_t _max_cmd = 32;   // Максимальное количество параметров.
		static const uint16_t _max_char = 512; // Максимальный размер строки с параметрами.
		char _beg_char = '$';                  // Начальный символ ($ или !).
		std::array<char, _max_char> _data;     // Данные.
		std::array<uint16_t, _max_cmd> _param; // Начала параметров в данных.
		state_enum _state = state_enum::NONE;
		uint8_t _crc = 0;
		uint16_t _idx_par = 0;
		uint16_t _idx_char = 0;
		size_t _data_idx = 0;

	public:
		static void calc_crc(char* buf, size_t len)
		{
			// $*00
			if (len < 4)
				return;
			const size_t max_i = len - 2;
			char crc = 0;
			size_t i = 1;
			for (; i < max_i; ++i)
			{
				if (buf[i] == '*')
					break;
				crc ^= buf[i];
			}
			if (i == max_i)
				return;
			buf[i + 1] = _hex[crc >> 4];
			buf[i + 2] = _hex[crc & 0xF];
		}

		NMEA()
		{
			// Первый параметр всегда в начале.
			_param[0] = 0;
		}

		// Начальный символ ($ или !).
		void set_beg_char(char beg_char)
		{
			if (beg_char == '$' || beg_char == '!')
				_beg_char = beg_char;
		}

		bool update(const uint8_t* data, const size_t size)
		{
			if (_state == state_enum::OK)
				_state = state_enum::NONE;
			else
				_data_idx = 0;
			for (; _data_idx < size; ++_data_idx)
			{
				const uint8_t v = data[_data_idx];
				// Непечатаемые символы.
				if (v < 32 || v > 126)
				{
					_state = state_enum::NONE;
					continue;
				}
				// Начало сообщения.
				if (v == _beg_char)
				{
					_crc = 0;
					_idx_par = 0;
					_idx_char = 0;
					_state = state_enum::BEG;
					continue;
				}
				switch (_state)
				{
					case state_enum::BEG:
					{
						switch (v)
						{
							case '*':
							{
								_data[_idx_char] = '\0';
								_idx_par++;
								_state = state_enum::CRC1;
								break;
							}
							case ',':
							{
								_crc ^= v;
								_data[_idx_char] = '\0';
								_idx_par++;
								_idx_char++;
								if (_idx_par >= _max_cmd || _idx_char >= _max_char)
									_state = state_enum::NONE;
								_param[_idx_par] = _idx_char;
								break;
							}
							default:
							{
								_crc ^= v;
								_data[_idx_char] = v;
								_idx_char++;
								if (_idx_char >= _max_char)
									_state = state_enum::NONE;
								break;
							}
						}
						break;
					}
					case state_enum::CRC1:
					{
						if (v == _hex[_crc >> 4])
							_state = state_enum::CRC2;
						else
							_state = state_enum::NONE;
						break;
					}
					case state_enum::CRC2:
					{
						if (v == _hex[_crc & 0xF])
						{
							_state = state_enum::OK;
							return true;
						}
						else
							_state = state_enum::NONE;
						break;
					}
					default:
						break;
				}
			}
			return false;
		}

		bool ok() const
		{
			return _state == state_enum::OK;
		}

		bool is_msg(const char* name, uint8_t size) const
		{
			if (_state != state_enum::OK)
				return false;
			if (_idx_par != size)
				return false;
			return strcmp(&_data[0], name) == 0;
		}

		uint16_t size() const
		{
			if (_state != state_enum::OK)
				return 0;
			return _idx_par;
		}

		bool is_name(uint16_t idx, const char* name) const
		{
			if (idx < _idx_par)
				return strcmp(&_data[_param[idx]], name) == 0;
			return false;
		}

		const char* get_str(uint16_t idx) const
		{
			if (idx < _idx_par)
				return &_data[_param[idx]];
			// Символ \0.
			return &_hex[16];
		}

		double get_double(uint16_t idx) const
		{
			if (idx < _idx_par)
				return std::atof(&_data[_param[idx]]);
			return 0.0;
		}

		int get_int(uint16_t idx) const
		{
			if (idx < _idx_par)
				return std::atoi(&_data[_param[idx]]);
			return 0;
		}

		uint16_t get_uint8(uint16_t idx) const
		{
			int val = get_int(idx);
			if (val < 0 || val > 255)
				return 0;
			return static_cast<uint16_t>(val);
		}
	};

	const char* NMEA::_hex = "0123456789ABCDEF";
}