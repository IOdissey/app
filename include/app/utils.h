// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace app
{
	void split(const std::string& s, const char* d, std::vector<std::string>& res)
	{
		res.clear();
		const size_t max = s.size() - 1;
		size_t beg = 0;
		size_t end;
		do
		{
			end = s.find_first_of(d, beg);
			if (end > beg)
				res.emplace_back(s.substr(beg, end - beg));
			else
				res.emplace_back("");
			beg = end + 1;
		}
		while (end < max);
	}

	// Получение текущего времени в виде строки.
	std::string time_str(const std::string& fmt)
	{
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, fmt.c_str());
		return oss.str();
	}

	template <typename T, size_t N>
	void print_arr(const std::array<T, N>& v)
	{
		if (N > 0)
		{
			std::cout << v[0];
			for (size_t i = 1; i < N; ++i)
				std::cout << " " << v[i];
		}
		std::cout << std::endl;
	}
}