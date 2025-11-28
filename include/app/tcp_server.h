// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <mutex>
#include <string>
#include <vector>
#define MG_ENABLE_LOG 0
#include <mongoose/mongoose.h>
#include "config.h"
#include "thread.h"


namespace app
{
	// WebSocket сервер.
	class TCPServer : public Thread
	{
	private:
		struct server_data_struct
		{
			std::vector<mg_connection*> tcp_arr; // Список подключений.
			bool is_read_data = false;
			std::string read_data;
			bool is_write_data = false;
			std::string write_data;
			std::mutex mutex;

			// Удаление подключения.
			void del(mg_connection* c)
			{
				const size_t len = tcp_arr.size();
				// Если нет ни одного ws подключения.
				if (len == 0)
					return;
				// Ищем какое подключение было завершено.
				size_t i = 0;
				for (; i < len; ++i)
				{
					if (tcp_arr[i] == c)
						break;
				}
				if (i >= len)
					return;
				for (size_t j = i + 1; j < len; ++j)
					tcp_arr[j - 1] = tcp_arr[j];
				tcp_arr.resize(len - 1);
				std::cout << "tcp clients: " <<  tcp_arr.size() << std::endl;
			}

			// Добавление подключения.
			void add(mg_connection* c)
			{
				tcp_arr.push_back(c);
				std::cout << "tcp clients: " <<  tcp_arr.size() << std::endl;
			}
		};

		bool _ok = false;
		mg_mgr _mgr;
		server_data_struct _server_data;
		int _min_ms = 5;
		std::string _read_data;

		static void _handler(mg_connection* c, int ev, void*, void* fn_data)
		{
			if (ev == MG_EV_ACCEPT)
			{
				server_data_struct* server_data = (server_data_struct*)fn_data;
				server_data->add(c);
			}
			else if (ev == MG_EV_CLOSE)
			{
				server_data_struct* server_data = (server_data_struct*)fn_data;
				server_data->del(c);
			}
			else if (ev == MG_EV_READ)
			{
				mg_iobuf* r = &c->recv;
				if (r->len < 1)
					return;
				server_data_struct* server_data = (server_data_struct*)fn_data;
				{
					std::lock_guard<std::mutex> guard(server_data->mutex);
					server_data->read_data += std::string((char*)r->buf, r->len);
					r->len = 0;
					server_data->is_read_data = true;
				}
			}
		}

		// Обработка в отдельном потоке.
		void _thread_run()
		{
			mg_mgr_poll(&_mgr, _min_ms);
			//
			const size_t len = _server_data.tcp_arr.size();
			if (len == 0 || !_server_data.is_write_data)
				return;
			std::lock_guard<std::mutex> guard(_server_data.mutex);
			_server_data.is_write_data = false;
			size_t data_len = _server_data.write_data.size();
			const char* data = _server_data.write_data.c_str();
			for (size_t i = 0; i < len; ++i)
				mg_send(_server_data.tcp_arr[i], data, data_len);
		}

	public:
		void end()
		{
			if (!_ok)
				return;
			thread_end();
			mg_mgr_free(&_mgr);
			_ok = false;
		}

		bool beg(const Config& cfg)
		{
			end();
			_min_ms = cfg.get("min_ms", 5);
			int port = cfg.get("port", 8089, 1, 65535);
			std::string url = "tcp://0.0.0.0:" + std::to_string(port);
			mg_mgr_init(&_mgr);
			mg_listen(&_mgr, url.c_str(), TCPServer::_handler, &_server_data);
			thread_run();
			_ok = true;
			return true;
		}

		bool is_read_data() const
		{
			return _server_data.is_read_data;
		}

		const std::string& read_data()
		{
			_read_data = "";
			std::lock_guard<std::mutex> guard(_server_data.mutex);
			_server_data.is_read_data = false;
			_read_data.swap(_server_data.read_data);
			return _read_data;
		}

		void write_data(const std::string& data)
		{
			std::lock_guard<std::mutex> guard(_server_data.mutex);
			_server_data.is_write_data = true;
			_server_data.write_data = data;
		}

		~TCPServer()
		{
			end();
		}
	};
}