// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include "utils.h"


namespace app
{
	class SimpleLog
	{
	private:
		std::ofstream _file;
		int _precision = 3;
		int _max_size = 100000000;
		std::string _affix = ".csv";

	public:
		bool beg(int max_size, int precision = 3, const std::string& affix = "")
		{
			_max_size = max_size;
			_precision = precision;
			_affix = affix + ".csv";
			return true;
		}

		bool open()
		{
			std::string path = time_str("logs/%Y-%m-%d/");
			if (!std::filesystem::exists(path))
				std::filesystem::create_directories(path);
			path += time_str("%H-%M-%S") + _affix;
			_file.open(path);
			if (!_file.good())
				return false;
			_file << std::fixed << std::showpoint << std::setprecision(_precision);
			return true;
		}

		void close()
		{
			if (_file.is_open())
				_file.close();
		}

		std::ofstream& get()
		{
			return _file;
		}

		void check_size()
		{
			if (_file.tellp() < _max_size)
				return;
			close();
			open();
		}
	};
}