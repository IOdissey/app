// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "config.h"
#include "imodule.h"
#include "print.h"
#include "rate.h"
#include "ws_server.h"


namespace app
{
	// Содержит список модулей для вызова.
	template <typename TState>
	class AppModule
	{
	private:
		app::Config _cfg;
		app::WSServer _ws_server;
		app::Json _json;
		app::Rate _rate;
		app::Rate _rate_send;
		std::vector<std::shared_ptr<IModule<TState>>> _modules;
		bool _debug = false;
		TState _state;

	public:
		virtual ~AppModule()
		{
		}

		app::Config& cfg()
		{
			return _cfg;
		}

		bool beg(int argc, char* argv[])
		{
			if (!_cfg.open(argc, argv))
			{
				app::print_error("Config not open");
				return false;
			}
			if (!_cfg.section("ws_server"))
				app::print_error("Section not found: ws_server");
			if (!_ws_server.beg(_cfg))
			{
				app::print_error("Module not started: ", "ws_server");
				return false;
			}
			_cfg.section("app");
			_rate.ms(_cfg.get<uint32_t>("period", 10));
			_rate_send.ms(_cfg.get<uint32_t>("period_send", 100));
			_debug = _cfg.get("debug", _debug);
			_state.ns = app::time::ns();
			_state.ms = static_cast<uint32_t>(_state.ns / 1000000);
			return true;
		}

		template <typename TModule>
		bool add(const std::string& name)
		{
			bool use = true;
			if (_cfg.section(name))
				use = _cfg.get<bool>("use", use);
			if (!use)
			{
				std::cout << "Module not used: " << name << std::endl;
				return false;
			}
			auto mod = std::make_shared<TModule>();
			if (!mod->beg(_cfg))
				return app::print_error("Module not started: ", name.c_str());
			// Добавление модуля.
			_modules.push_back(mod);
			return true;
		}

		virtual void send_data(app::Json&, TState&, bool)
		{
		}

		void update()
		{
			_rate.wait();
			uint64_t ns = app::time::ns();
			double dt = 1e-9 * static_cast<double>(ns - _state.ns);
			_state.ns = ns;
			_state.ms = static_cast<uint32_t>(_state.ns / 1000000);
			//
			const size_t size = _modules.size();
			for (size_t i = 0; i < size; ++i)
				_modules[i]->update(_state, dt);
			//
			if (_ws_server.is_json())
			{
				if (!_json.parse(_ws_server.get_json().c_str()))
					_json.print_error();
				else
				{
					for (size_t i = 0; i < size; ++i)
						_modules[i]->param(_json, _state);
				}
			}
			else if (_rate_send.ok())
			{
				bool new_connect = _ws_server.is_ws_new();
				_json.beg();
				send_data(_json, _state, new_connect);
				_ws_server.set_json(_json.end());
			}
		}

		void end()
		{
			const size_t size = _modules.size();
			for (size_t i = 0; i < size; ++i)
				_modules[i]->end();
		}
	};
}