// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


namespace app
{
	namespace _
	{
		template <typename T>
		bool bstot(const char* const str, T& val)
		{
			char* e;
			T v = static_cast<T>(std::strtoll(str, &e, 0));
			if (*e != '\0')
				return false;
			val = v;
			return true;
		}

		template <>
		bool bstot(const char* const str, double& val)
		{
			char* e;
			double v = std::strtod(str, &e);
			if (*e != '\0')
				return false;
			val = v;
			return true;
		}

		template <>
		bool bstot(const char* const str, float& val)
		{
			char* e;
			float v = std::strtof(str, &e);
			if (*e != '\0')
				return false;
			val = v;
			return true;
		}

		template <>
		bool bstot(const char* const str, bool& val)
		{
			char* e;
			int v = static_cast<int>(std::strtol(str, &e, 10));
			if (*e != '\0')
				return false;
			val = (v == 1);
			return true;
		}

		template <>
		bool bstot(const char* const str, std::string& val)
		{
			val = std::string(str);
			return true;
		}

		void print_beg(const std::string& name, bool offset = true)
		{
			if (offset)
				std::cout << "      \033[1;94m" << name << ": ";
			else
				std::cout << "   \033[1;94m" << name << ":";
		}

		void print_end(bool def)
		{
			if (def)
				std::cout << " \033[1;90m(missing)";
			std::cout << "\033[0m" << std::endl;
		}

		template <typename T>
		void print(const std::string& name, const T& val, bool def)
		{
			print_beg(name);
			std::cout << val;
			print_end(def);
		}

		template <>
		void print(const std::string& name, const uint8_t& val, bool def)
		{
			print_beg(name);
			std::cout << static_cast<int>(val);
			print_end(def);
		}

		template <typename T>
		void print_val(const T& val)
		{
			std::cout << val;
		}

		template <>
		void print_val(const uint8_t& val)
		{
			std::cout << static_cast<int>(val);
		}

		template <typename T>
		void print_vec(const std::string& name, const std::vector<T>& val, bool def)
		{
			print_beg(name);
			for (size_t i = 0; i < val.size(); ++i)
			{
				if (i > 0)
					std::cout << ", ";
				print_val(val[i]);
			}
			print_end(def);
		}

		template <typename T, uint8_t N>
		void print_arr(const std::string& name, const std::array<T, N>& val, bool def)
		{
			print_beg(name);
			for (uint8_t i = 0; i < N; ++i)
			{
				if (i > 0)
					std::cout << ", ";
				print_val(val[i]);
			}
			print_end(def);
		}

		void print_cstr(const std::string& name, const char* val, bool def)
		{
			print_beg(name);
			std::cout << val;
			print_end(def);
		}

		// Разбор строки на фрагменты.
		template <typename T>
		bool split(const char* str, std::vector<T>& item)
		{
			if (*str == '\0')
				return false;
			const uint8_t buf_size = 16;
			char buf[buf_size];
			T val;
			uint8_t i = 0;
			while (true)
			{
				const char c = *str;
				switch (c)
				{
					case '\0':
					case '\t':
					case ' ':
					case ',':
					case ';':
					{
						if (i > 0)
						{
							buf[i] = '\0';
							i = 0;
							if (!bstot<T>(buf, val))
								return false;
							item.emplace_back(val);
						}
						break;
					}
					default:
					{
						buf[i++] = c;
						if (i >= buf_size)
							return false;
						break;
					}
				}
				if (c == '\0')
					break;
				++str;
			}
			return true;
		}
	}

	class Config
	{
	private:
		enum class state_enum
		{
			IGNORE,
			PARAM_BEG,
			PARAM,
			VALUE_BEG,
			VALUE
		};

		struct Param
		{
			uint32_t beg;
			uint8_t size;
			uint32_t val;
		};

		std::vector<char> _buf;    // Содержит прочитанный файл.
		std::vector<Param> _param; // Список параметров.
		uint32_t _section = 0;     // Текущая секция.
		bool _use_print = true;    // Вывод прочитанных значений.

		// Разбор файла.
		void _parse()
		{
			state_enum state = state_enum::PARAM_BEG;
			Param param;
			uint32_t tmp;
			const uint32_t len = static_cast<uint32_t>(_buf.size());
			uint32_t i = 0;
			// BOM utf-8.
			if (static_cast<uint8_t>(_buf[0]) == 0xEF && static_cast<uint8_t>(_buf[1]) == 0xBB && static_cast<uint8_t>(_buf[2]) == 0xBF)
				i = 3;
			for (; i < len; ++i)
			{
				const char c = _buf[i];
				switch (state)
				{
					case state_enum::IGNORE:
					{
						if (c == '\n')
							state = state_enum::PARAM_BEG;
						break;
					}
					case state_enum::PARAM_BEG:
					{
						if (c < '_')
						{
							if (c < 'A')
							{
								if (c != ' ' && c != '\t' && c != '\n')
									state = state_enum::IGNORE;
								break;
							}
							if (c > 'Z')
							{
								state = state_enum::IGNORE;
								break;
							}
						}
						else if (c > 'z' || c == '`')
						{
							state = state_enum::IGNORE;
							break;
						}
						param.beg = i;
						state = state_enum::PARAM;
						break;
					}
					case state_enum::PARAM:
					{
						if (c < 'A')
						{
							if (c < '-')
							{
								if (c == '\n')
									state = state_enum::PARAM_BEG;
								else
									state = state_enum::IGNORE;
								break;
							}
							if (c > ':' || c == '/')
							{
								state = state_enum::IGNORE;
								break;
							}
							if (c == ':')
							{
								_buf[i] = '\0';
								param.size = static_cast<uint8_t>(i - param.beg);
								state = state_enum::VALUE_BEG;
								break;
							}
						}
						else if (c >= '_')
						{
							if (c > 'z' || c == '`')
							{
								state = state_enum::IGNORE;
								break;
							}
						}
						else if (c > 'Z')
						{
							state = state_enum::IGNORE;
							break;
						}
						break;
					}
					case state_enum::VALUE_BEG:
					{
						if (c == ' ' || c == '\t')
							break;
						if (c > ' ' && c <= '~' && c != '#')
						{
							param.val = i;
							tmp = 0;
							state = state_enum::VALUE;
							break;
						}
						if (c == '\n')
							state = state_enum::PARAM_BEG;
						else
							state = state_enum::IGNORE;
						// Проверка, что это секция (имя параметра в начале строки).
						if (param.beg == 0 || _buf[param.beg - 1] == '\n')
							param.val = 0;
						else
						{
							_buf[i] = '\0';
							param.val = i;
						}
						_param.push_back(param);
						break;
					}
					case state_enum::VALUE:
					{
						if (c == ' ' || c == '\t')
						{
							if (tmp == 0)
								tmp = i;
							break;
						}
						if (c < ' ' || c > '~' || (tmp > 0 && c == '#'))
						{
							if (tmp > 0)
								_buf[tmp] = '\0';
							else
								_buf[i] = '\0';
							_param.push_back(param);
							if (c == '\n')
								state = state_enum::PARAM_BEG;
							else
								state = state_enum::IGNORE;
							break;
						}
						if (tmp > 0)
							tmp = 0;
						break;
					}
				}
			}
		}

		// Чтение файла.
		bool _open(const std::string& name)
		{
			_buf.clear();
			std::ifstream file(name, std::ios_base::binary);
			if (!file)
				return false;
			file.seekg(0, std::ios_base::end);
			const size_t len = static_cast<size_t>(file.tellg());
			if (len < 3)
				return false;
			file.seekg(0, std::ios_base::beg);
			_buf.resize(len + 1);
			file.read(&_buf[0], len);
			_buf[len] = '\0';
			return true;
		}

		// Поиск индекса параметра.
		uint32_t _idx_val(const std::string& name) const
		{
			if (_section == 0)
				return 0;
			const uint32_t size = static_cast<uint32_t>(_param.size());
			const uint8_t len = static_cast<uint8_t>(name.size());
			const char* const str = name.c_str();
			for (uint32_t i = _section; i < size; ++i)
			{
				const auto& param = _param[i];
				if (param.val == 0)
					break;
				if (param.size != len)
					continue;
				if (std::strcmp(str, &_buf[param.beg]) == 0)
					return param.val;
			}
			return 0;
		}

	public:
		bool open(const std::string& name)
		{
			if (!_open(name))
			{
				std::cout << "\033[1;31mError. Config open: " << name << ".\033[0m" << std::endl;
				return false;
			}
			_parse();
			return true;
		}

		bool open(int argc, char* argv[])
		{
			if (argc < 2)
				return open("config.yml");
			else
				return open(argv[1]);
		}

		bool use_print() const
		{
			return _use_print;
		}

		void use_print(bool use)
		{
			_use_print = use;
		}

		bool section(const std::string& name)
		{
			const uint32_t size = static_cast<uint32_t>(_param.size());
			const uint8_t len = static_cast<uint8_t>(name.size());
			const char* const str = name.c_str();
			for (uint32_t i = 0; i < size; ++i)
			{
				const auto& param = _param[i];
				if (param.val != 0 || param.size != len)
					continue;
				if (std::strcmp(str, &_buf[param.beg]) == 0)
				{
					_section = i + 1;
					if (_use_print)
					{
						_::print_beg(name, false);
						_::print_end(false);
					}
					return true;
				}
			}
			_section = 0;
			if (_use_print)
			{
				_::print_beg(name, false);
				_::print_end(true);
			}
			return false;
		}

		const Config& operator()(const std::string& name)
		{
			section(name);
			return *this;
		}

		template <typename T>
		T get(const std::string& name, const T& def) const
		{
			T val;
			uint32_t idx = _idx_val(name);
			if (idx == 0 || !_::bstot(&_buf[idx], val))
			{
				if (_use_print)
					_::print(name, def, true);
				return def;
			}
			if (_use_print)
				_::print(name, val, false);
			return val;
		}

		const char* get_cstr(const std::string& name, const char* def) const
		{
			const char* val;
			const uint32_t idx = _idx_val(name);
			if (idx == 0)
			{
				if (_use_print)
					_::print_cstr(name, def, true);
				return def;
			}
			else
			{
				val = &_buf[idx];
				if (_use_print)
					_::print_cstr(name, val, false);
				return val;
			}
		}

		template <typename T>
		T get(const std::string& name, const T& def, const T& val_min, const T& val_max) const
		{
			T val;
			uint32_t idx = _idx_val(name);
			if (idx == 0 || !_::bstot(&_buf[idx], val) || val < val_min || val > val_max)
			{
				if (_use_print)
					_::print(name, def, true);
				return def;
			}
			if (_use_print)
				_::print(name, val, false);
			return val;
		}

		template <typename T>
		std::vector<T> get_vec(const std::string& name, const std::vector<T>& def = std::vector<T>()) const
		{
			std::vector<T> val;
			const uint32_t idx = _idx_val(name);
			if (idx == 0 || !_::split<T>(&_buf[idx], val))
			{
				if (_use_print)
					_::print_vec<T>(name, def, true);
				return def;
			}
			if (_use_print)
				_::print_vec<T>(name, val, false);
			return val;
		}

		template <typename T, uint8_t N>
		std::array<T, N> get_arr(const std::string& name, const std::array<T, N>& def = {}) const
		{
			std::array<T, N> val;
			const uint32_t idx = _idx_val(name);
			if (idx != 0)
			{
				std::vector<T> items;
				if (_::split<T>(&_buf[idx], items))
				{
					const uint8_t size = static_cast<uint8_t>(items.size());
					if (size == N)
					{
						for (uint8_t i = 0; i < size; ++i)
							val[i] = items[i];
						if (_use_print)
							_::print_arr<T, N>(name, val, false);
						return val;
					}
				}
			}
			if (_use_print)
				_::print_arr<T, N>(name, def, true);
			return def;
		}

		// Get enumerate.
		template <typename T, uint8_t N>
		T get(const std::string& name, const T& def, const std::array<T, N>& list) const
		{
			int i = get<int>(name, -1);
			if (i < 0 || i >= N)
				return def;
			return list[i];
		}
	};
}