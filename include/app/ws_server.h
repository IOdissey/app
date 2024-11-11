// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
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
	class WSServer : public Thread
	{
	private:
		struct server_data_struct
		{
			std::vector<mg_connection*> ws_arr; // Список подключений.
			bool is_ws_new = false;             // Флаг нового подключения.
			std::string json_get;               // Полученные данные.
			std::string json_set;               // Данные для отправки.
			volatile bool is_get_json = false;
			volatile bool is_set_json = false;
			std::mutex mutex;

			// Удаление подключения.
			void del(mg_connection* c)
			{
				const size_t len = ws_arr.size();
				// Если нет ни одного ws подключения.
				if (len == 0)
					return;
				// Ищем какое подключение было завершено.
				size_t i = 0;
				for (; i < len; ++i)
				{
					if (ws_arr[i] == c)
						break;
				}
				if (i >= len)
					return;
				for (size_t j = i + 1; j < len; ++j)
					ws_arr[j - 1] = ws_arr[j];
				ws_arr.resize(len - 1);
				std::cout << "ws count: " <<  ws_arr.size() << std::endl;
			}

			// Добавление подключения.
			void add(mg_connection* c)
			{
				ws_arr.push_back(c);
				std::cout << "ws count: " <<  ws_arr.size() << std::endl;
				if (!is_ws_new)
				{
					std::lock_guard<std::mutex> guard(mutex);
					is_ws_new = true;
				}
			}
		};

		bool _ok = false;
		mg_mgr _mgr;
		server_data_struct _server_data;
		int _min_ms = 5;

		static void _request_handler(mg_connection* c, int ev, void* ev_data, void* fn_data)
		{
			if (ev == MG_EV_CLOSE)
			{
				server_data_struct* server_data = (server_data_struct*)fn_data;
				server_data->del(c);
			}
			else if (ev == MG_EV_HTTP_MSG)
			{
				mg_http_message* http_msg = (mg_http_message*)ev_data;
				if (mg_http_match_uri(http_msg, "/ws"))
				{
					std::cout << "http: /ws" << std::endl;
					server_data_struct* server_data = (server_data_struct*)fn_data;
					server_data->add(c);
					mg_ws_upgrade(c, http_msg, nullptr);
				}
				else if (mg_http_match_uri(http_msg, "/ok"))
				{
					std::cout << "http: /ok" << std::endl;
					mg_http_reply(c, 200, "Content-Type: application/json; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\n", "{\"status\": \"ok\"}\n");
				}
			}
			else if (ev == MG_EV_WS_MSG)
			{
				mg_ws_message* wm = (mg_ws_message*)ev_data;
				server_data_struct* server_data = (server_data_struct*)fn_data;
				if (wm->data.len < 1)
					return;
				{
					std::lock_guard<std::mutex> guard(server_data->mutex);
					server_data->json_get = std::string(wm->data.ptr, wm->data.len);
					server_data->is_get_json = true;
				}
				mg_iobuf_del(&c->recv, 0, c->recv.len);
			}
		}

		// Обработка http в отдельном потоке.
		void _thread_run()
		{
			mg_mgr_poll(&_mgr, _min_ms);
			//
			const size_t len = _server_data.ws_arr.size();
			if (len == 0 || !_server_data.is_set_json)
				return;
			std::lock_guard<std::mutex> guard(_server_data.mutex);
			_server_data.is_set_json = false;
			size_t data_len = _server_data.json_set.size();
			const char* data = _server_data.json_set.c_str();
			for (size_t i = 0; i < len; ++i)
				mg_ws_send(_server_data.ws_arr[i], data, data_len, WEBSOCKET_OP_TEXT);
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
			int port = cfg.get("port", 8080, 1, 65535);
			mg_mgr_init(&_mgr);
			std::string url = "http://0.0.0.0:" + std::to_string(port);
			mg_http_listen(&_mgr, url.c_str(), WSServer::_request_handler, &_server_data);
			thread_run();
			_ok = true;
			return true;
		}

		~WSServer()
		{
			end();
		}

		bool is_json() const
		{
			return _server_data.is_get_json;
		}

		std::string get_json()
		{
			std::lock_guard<std::mutex> guard(_server_data.mutex);
			_server_data.is_get_json = false;
			return _server_data.json_get;
		}

		bool is_ws_new()
		{
			if (!_server_data.is_ws_new)
				return false;
			std::lock_guard<std::mutex> guard(_server_data.mutex);
			_server_data.is_ws_new = false;
			return true;
		}

		void set_json(const std::string& json)
		{
			std::lock_guard<std::mutex> guard(_server_data.mutex);
			_server_data.is_set_json = true;
			_server_data.json_set = json;
		}
	};
}