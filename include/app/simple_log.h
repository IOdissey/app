// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include "config.h"
#include "utils.h"


namespace app
{
	class SimpleLog
	{
	private:
		std::ofstream _file;
		int _precision = 3;
		int _max_size = 100000000;
		std::string _folder = "logs";

	public:
		bool beg(const app::Config& cfg)
		{
			_max_size = cfg.get("max_size", _max_size);
			_precision = cfg.get("precision", _precision);
			_folder = cfg.get<std::string>("folder", _folder);
			return open();
		}

		bool open()
		{
			std::string path = time_str(_folder + "/%Y-%m-%d/");
			if (!std::filesystem::exists(path))
				std::filesystem::create_directories(path);
			path += time_str("%H-%M-%S.csv");
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