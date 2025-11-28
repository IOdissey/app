// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <arpa/inet.h>
#include <array>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "print.h"
#include "time.h"


namespace app
{
	class TCPClient
	{
	private:
		int _sock = -1;
		sockaddr_in _addr;             // Адрес сервера.
		uint32_t _reconnect_ms = 0;    // Время переподключения.
		uint32_t _last_connect_ms = 0; // Время попытки подключения.
		std::vector<uint8_t> _buf;     // Буфер данных.
		size_t _data_size = 0;         // Количество прочитанных данных.

		// Подключение к заданному серверу.
		bool _connect()
		{
			_last_connect_ms = time::ms() + _reconnect_ms;
			_sock = socket(AF_INET, SOCK_STREAM, 0);
			if (_sock < 0)
				return print_errno("TCPClient socket");
			if (connect(_sock, (sockaddr*)&_addr, sizeof(_addr)) != 0)
			{
				close(_sock);
				_sock = -1;
				return print_errno("TCPClient connect");
			}
			return true;
		}

	public:
		TCPClient()
		{
			memset(&_addr, 0, sizeof(sockaddr_in));
			_addr.sin_family = AF_INET;
		}

		~TCPClient()
		{
			end();
		}

		void end()
		{
			if (_sock < 0)
				return;
			close(_sock);
			_sock = -1;
		}

		bool beg(const app::Config& cfg)
		{
			_addr.sin_addr.s_addr = inet_addr(cfg.get_cstr("host", "127.0.0.1"));
			_addr.sin_port = htons(cfg.get<uint16_t>("port", 8000));
			_reconnect_ms = cfg.get<uint32_t>("reconnect_ms", 0);
			_buf.resize(cfg.get<uint32_t>("buf_size", 1024));
			if (_sock >= 0)
				close(_sock);
			bool ok = _connect();
			if (_reconnect_ms > 0)
				return true;
			return ok;
		}

		bool send_data(const char* const data, size_t size)
		{
			int res = static_cast<int>(send(_sock, data, (size_t)size, MSG_NOSIGNAL));
			if (res < 0)
				return print_errno("TCPClient send");
			return true;
		}

		bool send_data(const uint8_t* const data, size_t size)
		{
			return send_data((const char*)data, size);
		}

		bool send_data(const std::string& str)
		{
			return send_data(str.data(), str.size());
		}

		// TODO. Обработка ошибок.
		bool update()
		{
			// Попытка переподключения.
			if (_sock < 0)
			{
				if (_reconnect_ms == 0)
					return false;
				uint32_t now = time::ms();
				if (now < _last_connect_ms)
					return false;
				if (!_connect())
					return false;
			}
			const auto data_size = recv(_sock, _buf.data(), _buf.size(), MSG_DONTWAIT);
			if (data_size < 0)
			{
				if (errno != 11)
				{
					print_errno("TCPClient recv");
					end();
				}
				return false;
			}
			_data_size = data_size;
			return _data_size > 0;
		}

		const uint8_t* data() const
		{
			return _buf.data();
		}

		size_t data_size() const
		{
			return _data_size;
		}
	};
}