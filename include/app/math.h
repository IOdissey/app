// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
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

	// http://www.movable-type.co.uk/scripts/latlong.html
	// TODO. Проверка для корня.
	void geo_inverse(double lat1, double lon1, double lat2, double lon2, double& m_lat, double& m_lon)
	{
		// Радиус Земли в метрах (удвоенный).
		constexpr double r = 2.0 * 6378137.0;
		// Переводим в радианы.
		lat1 *= DEG_RAD;
		lon1 *= DEG_RAD;
		lat2 *= DEG_RAD;
		lon2 *= DEG_RAD;
		double d_lat = (lat2 - lat1) * 0.5;
		double d_lon = (lon2 - lon1) * 0.5;
		// double a = std::sin(d_lat) * std::sin(d_lat) + std::cos(lat1) * std::cos(lat2) * std::sin(d_lon) * std::sin(d_lon);
		// d_lon = 0
		// Для небольших перемещений.
		// |d_lat| << 90 градусов => |sin_d_lat| < 1.0
		double sin_d_lat = std::sin(d_lat);
		double a_lat = sin_d_lat * sin_d_lat;
		m_lat = r * std::atan(std::sqrt(a_lat / (1.0 - a_lat)));
		if (d_lat < 0)
			m_lat = -m_lat;
		// d_lat = 0
		// Для небольших перемещений.
		// |d_lon| << 90 градусов => |sin_d_lon| < 1.0
		double sin_d_lon = std::sin(d_lon);
		double a_lon = std::cos(lat1) * std::cos(lat2) * sin_d_lon * sin_d_lon;
		m_lon = r * std::atan(std::sqrt(a_lon / (1.0 - a_lon)));
		if (d_lon < 0)
			m_lon = -m_lon;
	}
}