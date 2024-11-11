// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <array>
#include <cmath>
#include <fstream>
#include <vector>
#define NDEBUG
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "math.h"


namespace app
{
	class Json
	{
	private:
		rapidjson::Document _json;
		rapidjson::StringBuffer _buffer;
		rapidjson::Writer<rapidjson::StringBuffer> _writer;
		rapidjson::Value* _section = nullptr;               // Текущая секция для чтения.
		rapidjson::ParseResult _ok;

		// Получение значения.
		template <typename T>
		bool _get(const rapidjson::Value& json_val, T& val) const
		{
			if (!json_val.IsNumber())
				return false;
			val = json_val.Get<T>();
			return true;
		}

		// Получение значения.
		bool _get(const rapidjson::Value& json_val, bool& val) const
		{
			if (!json_val.IsBool())
				return false;
			val = json_val.Get<bool>();
			return true;
		}

		// Получение значения.
		bool _get(const rapidjson::Value& json_val, std::string& val) const
		{
			if (!json_val.IsString())
				return false;
			val = json_val.GetString();
			return true;
		}

		// Получение списка значений из класса.
		template <typename T>
		bool _get(const rapidjson::Value& json_val, std::vector<std::pair<std::string, T>>& val) const
		{
			if (!json_val.IsObject())
				return false;
			const size_t size = json_val.MemberCount();
			if (size == 0)
				return false;
			val.clear();
			val.reserve(size);
			for (rapidjson::Value::ConstMemberIterator it = json_val.MemberBegin(); it != json_val.MemberEnd(); ++it)
				val.emplace_back(std::make_pair<std::string, T>(it->name.GetString(), it->value.Get<T>()));
			return true;
		}

		// Получение списка значений из массива.
		template <typename T>
		bool _get(const rapidjson::Value& json_val, std::vector<T>& val) const
		{
			if (!json_val.IsArray())
				return false;
			const size_t size = json_val.Size();;
			if (size == 0)
				return false;
			val.clear();
			val.reserve(size);
			for (rapidjson::Value::ConstValueIterator itr = json_val.Begin(); itr != json_val.End(); ++itr)
				val.push_back(itr->Get<T>());
			return true;
		}

		// Получение списка значений из массива.
		template <typename T, size_t N>
		bool _get(const rapidjson::Value& json_val, std::array<T, N>& val) const
		{
			if (!json_val.IsArray())
				return false;
			const size_t size = json_val.Size();;
			if (size != N)
				return false;
			size_t i = 0;
			for (rapidjson::Value::ConstValueIterator itr = json_val.Begin(); itr != json_val.End(); ++itr)
				val[i++] = itr->Get<T>();
			return true;
		}

		// Задание переменных.
		void _set(const char* name, rapidjson::Value& val, const bool key_copy = false)
		{
			if (key_copy)
				_section->AddMember(rapidjson::Value(name, _json.GetAllocator()), val, _json.GetAllocator());
			else
				_section->AddMember(rapidjson::StringRef(name), val, _json.GetAllocator());
		}

		void _round(double& val, const int p) const
		{
			val = math::round(val, p);
		}

		template <size_t N>
		void _round(std::array<double, N>& val, const int p) const
		{
			for (size_t i = 0; i < N; ++i)
				val[i] = math::round(val[i], p);
		}

		void _round(std::vector<double>& val, const int p) const
		{
			size_t n = val.size();
			for (size_t i = 0; i < n; ++i)
				val[i] = math::round(val[i], p);
		}

	public:
		// decimal - количество знаков после запятой.
		Json(int decimal = 9) :
			_writer(_buffer)
		{
			set_decimal(decimal);
		}

		// decimal - количество знаков после запятой.
		void set_decimal(int decimal)
		{
			_writer.SetMaxDecimalPlaces(decimal);
		}

		// Разбор строки данных.
		// data - строка данных.
		// В случае успеха возвращает true.
		bool parse(const char* data)
		{
			_ok = _json.Parse(data);
			if (!_ok)
				return false;
			return get("");
		}

		// Разбор строки данных.
		// data - строка данных.
		// В случае успеха возвращает true.
		bool parse(const char* data, size_t len)
		{
			_ok = _json.Parse(data, len);
			if (!_ok)
				return false;
			return get("");
		}

		// Разбор данных из файла.
		// file - наименование файла.
		// В случае успеха возвращает true.
		bool parse_file(const std::string& file)
		{
			std::ifstream ifs(file);
			rapidjson::IStreamWrapper isw(ifs);
			_ok = _json.ParseStream(isw);
			if (!_ok)
				return false;
			return get("");
		}

		void print_error()
		{
			std::cout << "JSON parse error: " << GetParseError_En(_ok.Code()) << " Offset: " << _ok.Offset() << "." << std::endl;
		}

		// Есть ли данное поле.
		bool has(const char* name) const
		{
			if (!_section)
				return false;
			return _section->HasMember(name);
		}

		// Выбор секции.
		bool get(const char* name)
		{
			_section = rapidjson::Pointer(name).Get(_json);
			if (!_section || _section->IsNull())
				return false;
			return true;
		}

		// Получение значения.
		template <typename T>
		bool get(const char* name, T& val) const
		{
			if (!_section)
				return false;
			const auto it = _section->FindMember(name);
			if (it == _section->MemberEnd())
				return false;
			return _get(it->value, val);
		}

		// Получение значения и проверка, изменилось ли оно.
		template <typename T>
		bool get_change(const char* name, T& val) const
		{
			if (!_section)
				return false;
			const auto it = _section->FindMember(name);
			if (it == _section->MemberEnd())
				return false;
			T old_val = val;
			if (!_get(it->value, val))
				return false;
			return old_val != val;
		}

		// Проверка bool значения.
		bool is(const char* name)
		{
			bool b = false;
			get(name, b);
			return b;
		}

		// Начало формирования json.
		void beg()
		{
			_json.SetObject();
			set("");
		}

		// Окончание формирования json. 
		std::string end()
		{
			_buffer.Clear();
			_writer.Reset(_buffer);
			_json.Accept(_writer);
			return std::string(_buffer.GetString(), _buffer.GetSize());
		}

		// Окончание формирования json. 
		const char* end(size_t& len)
		{
			_buffer.Clear();
			_writer.Reset(_buffer);
			_json.Accept(_writer);
			len = _buffer.GetSize();
			return _buffer.GetString();
		}

		// Добавление секции.
		void set(const char* name)
		{
			_section = rapidjson::Pointer(name).Get(_json);
			if (!_section || _section->IsNull())
			{
				_section = &(rapidjson::Pointer(name).Create(_json));
				_section->SetObject();
			}
		}

		// Задание массива.
		template <typename T>
		void set(const char* name, const std::vector<T>& val, const bool key_copy = false)
		{
			rapidjson::Value array(rapidjson::kArrayType);
			for (size_t i = 0; i < val.size(); ++i)
				array.PushBack(val[i], _json.GetAllocator());
			if (key_copy)
				_section->AddMember(rapidjson::Value(name, _json.GetAllocator()), array, _json.GetAllocator());
			else
				_section->AddMember(rapidjson::StringRef(name), array, _json.GetAllocator());
		}

		// Задание массива.
		template <typename T, size_t N>
		void set(const char* name, const std::array<T, N>& val, const bool key_copy = false)
		{
			rapidjson::Value array(rapidjson::kArrayType);
			for (size_t i = 0; i < N; ++i)
				array.PushBack(val[i], _json.GetAllocator());
			_set(name, array, key_copy);
		}

		// Задание массива.
		void set(const char* name, const std::vector<std::string>& val, const bool val_copy = false, const bool key_copy = false)
		{
			rapidjson::Value array(rapidjson::kArrayType);
			if (val_copy)
			{
				for (size_t i = 0; i < val.size(); ++i)
					array.PushBack(rapidjson::Value(val[i].c_str(), _json.GetAllocator()), _json.GetAllocator());
			}
			else
			{
				for (size_t i = 0; i < val.size(); ++i)
					array.PushBack(rapidjson::StringRef(val[i].c_str()), _json.GetAllocator());
			}
			_set(name, array, key_copy);
		}

		// Задание переменных.
		template <typename T>
		void set(const char* name, const T& val, const bool key_copy = false)
		{
			if (key_copy)
				_section->AddMember(rapidjson::Value(name, _json.GetAllocator()), rapidjson::Value(val), _json.GetAllocator());
			else
				_section->AddMember(rapidjson::StringRef(name), val, _json.GetAllocator());
		}

		// Значение с округлением.
		// precision = [0, 9], иначе ошибка.
		template <typename T>
		void set(const char* name, T val, int precision, const bool key_copy = false)
		{
			_round(val, precision);
			set(name, val, key_copy);
		}

		// Задание массива.
		void set_arr(const char* name, const double* val, size_t size, const bool key_copy = false)
		{
			rapidjson::Value array(rapidjson::kArrayType);
			for (size_t i = 0; i < size; ++i)
				array.PushBack(val[i], _json.GetAllocator());
			if (key_copy)
				_section->AddMember(rapidjson::Value(name, _json.GetAllocator()), array, _json.GetAllocator());
			else
				_section->AddMember(rapidjson::StringRef(name), array, _json.GetAllocator());
		}
	};
}