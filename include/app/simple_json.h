// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <array>
#include <cstdio>
#include <string>
#include <vector>


namespace app
{
	// Кавычки (") в строках не экранируются.
	class SimpleJson
	{
	private:
		std::vector<char> _buf;
		int _buf_size = 2048;
		int _buf_idx = 0;

		void _comma()
		{
			if (_buf[_buf_idx - 1] == '{')
				return;
			_buf[_buf_idx] = ',';
			++_buf_idx;
		}

	public:
		SimpleJson()
		{
			_buf.resize(_buf_size);
		}

		void add()
		{
			_buf[_buf_idx] = '{';
			_buf_idx = 1;
		}

		void add(const char* name)
		{
			_comma();
			_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":{", name);
		}

		void add(const char* name, int val)
		{
			_comma();
			_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%d", name, val);
		}

		void add(const char* name, double val, int precision = 3)
		{
			_comma();
			switch (precision)
			{
				case 1:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%.1f", name, val);
					break;
				case 2:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%.2f", name, val);
					break;
				case 3:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%.3f", name, val);
					break;
				case 4:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%.4f", name, val);
					break;
				case 5:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%.5f", name, val);
					break;
				case 6:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%.6f", name, val);
					break;
				case 7:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%.7f", name, val);
					break;
				default:
					_buf_idx += sprintf(&_buf[_buf_idx], "\"%s\":%f", name, val);
					break;
			}
		}

		void end()
		{
			_buf[_buf_idx] = '}';
			++_buf_idx;
		}

		std::string str()
		{
			return std::string(_buf.data(), _buf_idx);
		}
	};
}