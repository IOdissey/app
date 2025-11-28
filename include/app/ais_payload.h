// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <limits>
#include <string>
#include <vector>


namespace app
{
	// https://gpsd.gitlab.io/gpsd/AIVDM.html
	class AISPayload
	{
	private:
		//
		static constexpr double _lon_lat_scale = 1.0 / 600000.0;
		static constexpr double _no_data = std::numeric_limits<int32_t>::max();

		std::vector<uint8_t> _data;
		uint16_t _data_bit = 0;
		uint8_t _type = 0;

		int32_t _get_int(uint16_t beg_bit, uint16_t len, bool sign = false) const
		{
			const uint16_t end_bit = beg_bit + len;
			if (end_bit >= _data_bit)
				return _no_data;
			int32_t res = 0;
			for (uint16_t i = beg_bit; i < end_bit; ++i)
			{
				res <<= 1;
				uint8_t byte = _data[i / 6];
				uint8_t bit = (byte >> (5 - (i % 6))) & 1;
				if (bit == 0)
					continue;
				if (i == beg_bit && sign)
					res = ~res;
				res |= 1;
			}
			return res;
		}

		// Longitude is given in in 1/10000 min; divide by 600000.0 to obtain degrees.
		// Values up to plus or minus 180 degrees, East = positive, West = negative.
		// A value of 181 degrees (0x6791AC0 hex) indicates that longitude is not available and is the default.
		// Результат в градусах.
		bool _get_lon(uint16_t beg_bit, double& val) const
		{
			int ival = _get_int(beg_bit, 28, true);
			if (ival == _no_data || ival == 0x6791AC0)
				return false;
			val = ival * _lon_lat_scale;
			return true;
		}

		// Latitude is given in in 1/10000 min; divide by 600000.0 to obtain degrees.
		// Values up to plus or minus 90 degrees, North = positive, South = negative.
		// A value of 91 degrees (0x3412140 hex) indicates latitude is not available and is the default.
		// Результат в градусах.
		bool _get_lat(uint16_t beg_bit, double& val) const
		{
			int ival = _get_int(beg_bit, 27, true);
			if (ival == _no_data || ival == 0x3412140)
				return false;
			val = ival * _lon_lat_scale;
			return true;
		}

		// Speed Over Ground (SOG).
		// Результат в м/с.
		bool _get_sog(uint16_t beg_bit, double& val) const
		{
			int ival = _get_int(beg_bit, 10);
			if (ival == _no_data || ival == 1023)
				return false;
			val = ival * 0.0514444;
			return true;
		}

		// Course Over Ground (COG).
		// Результат в градусах.
		bool _get_cog(uint16_t beg_bit, double& val) const
		{
			int ival = _get_int(beg_bit, 12);
			if (ival == _no_data || ival == 3600)
				return false;
			val = ival * 0.1;
			return true;
		}

		// True Heading (HDG).
		// Результат в градусах.
		bool _get_hdg(uint16_t beg_bit, double& val) const
		{
			int ival = _get_int(beg_bit, 9);
			if (ival == _no_data || ival == 511)
				return false;
			val = ival;
			return true;
		}

	public:
		AISPayload(const std::string& data)
		{
			const size_t size = data.size();
			_data_bit = static_cast<uint16_t>(size * 6);
			_data.resize(size);
			for (size_t i = 0; i < size; ++i)
			{
				_data[i] = static_cast<uint8_t>(data[i] - 48);
				if (_data[i] > 40)
					_data[i] -= 8;
			}
			_type = _data[0];
		};

		uint8_t get_type() const
		{
			return _type;
		}

		int get_mmsi() const
		{
			return _get_int(8, 30);
		}

		// Данные из сообщений с позицией (1, 2, 3, 18, 19).
		bool get_position_report(double& lat, double& lon, double& sog, double& cog, double& hdg) const
		{
			if (_type > 0 && _type < 4)
			{
				_get_sog(50, sog);
				_get_lon(61, lon);
				_get_lat(89, lat);
				_get_cog(116, cog);
				_get_hdg(128, hdg);
				return true;
			}
			if (_type == 18 || _type == 19)
			{
				_get_sog(46, sog);
				_get_lon(57, lon);
				_get_lat(85, lat);
				_get_cog(112, cog);
				_get_hdg(124, hdg);
				return true;
			}
			return false;
		}
	};
}