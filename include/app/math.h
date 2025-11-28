// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <array>
#include <cmath>


namespace math
{
	namespace _
	{
		const std::array<double, 10> factor = {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9};
	}

	constexpr double PI = 3.141592653589793238463;
	constexpr double PI2 = 2 * PI;
	constexpr double RAD_DEG = 180.0 / PI;
	constexpr double DEG_RAD = PI / 180.0;

	double round(double val, const int p)
	{
		return std::round(val * _::factor[p]) / _::factor[p];
	}

	template <typename T>
	T limit(T v, T v_min, T v_max)
	{
		if (v > v_max)
			return v_max;
		else if (v < v_min)
			return v_min;
		return v;
	}

	template <typename T>
	T limit(T v, T v_max)
	{
		if (v > v_max)
			return v_max;
		else if (v < -v_max)
			return -v_max;
		return v;
	}

	// Нормировка угла.
	double angle(double a)
	{
		if (a > PI || a < -PI)
		{
			a = std::fmod(a + PI, PI2);
			if (a < 0.0)
				return a + PI;
			return a - PI;
		}
		return a;
	}

	// Нормировка угла (градусы).
	double angle_deg(double a)
	{
		if (a > 180.0 || a < -180.0)
		{
			a = std::fmod(a + 180.0, 360.0);
			if (a < 0.0)
				return a + 180.0;
			return a - 180.0;
		}
		return a;
	}
}