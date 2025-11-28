// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <array>
#include "math.h"


namespace geo
{
	namespace _
	{
		double lat_m = 0.0;
		double lon_m = 0.0;
		double lat_0 = 0.0;

		void calc(double lat, double thr)
		{
			// Не пересчитываем коэффициенты, если расхождение меньше порога thr.
			if (std::abs(lat - lat_0) < thr)
				return;
			// https://en.wikipedia.org/wiki/Geographic_coordinate_system#Latitude_and_longitude
			const double a = lat * math::DEG_RAD;
			lat_m = 111132.92 - 559.82 * std::cos(2.0 * a) + 1.175 * std::cos(4.0 * a);
			lon_m = 111412.84 * std::cos(a) - 93.5 * std::cos(3.0 * a) + 0.118 * std::cos(5.0 * a);
			lat_0 = lat;
		}
	}

	void inverse(double lat1, double lon1, double lat2, double lon2, double& m_lat, double& m_lon, double thr = 0.1)
	{
		_::calc((lat1 + lat2) * 0.5, thr);
		m_lat = (lat2 - lat1) * _::lat_m;
		m_lon = (lon2 - lon1) * _::lon_m;
	}

	void move(double lat1, double lon1, double m_lat, double m_lon, double& lat2, double& lon2, double thr = 0.1)
	{
		_::calc(lat1, thr);
		lat2 = lat1 + m_lat / _::lat_m;
		lon2 = lon1 + m_lon / _::lon_m;
	}
}