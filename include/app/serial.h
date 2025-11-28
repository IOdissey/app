// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <string>
#include <functional>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>


namespace app
{
	class Serial
	{
	private:
		int _fd = -1;          // Файловый дескриптор.

	public:
		enum class Parity
		{
			NO,
			EVEN,
			ODD
		};

		bool serial_open(const std::string& port, int baud = 9600, int bits = 8, Parity parity = Parity::NO, bool stopbit = false)
		{
			_fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
			// Check opening status
			if (_fd < 0)
				return false;
			// Get terminal parameters
			struct termios param;
			tcgetattr(_fd, &param);
			// Flushes data received but not read
			ioctl(_fd, TCIFLUSH);
			// Set baud rate (in and out)
			switch (baud)
			{
				case 9600:
					cfsetspeed(&param, B9600);
					break;
				case 19200:
					cfsetspeed(&param, B19200);
					break;
				case 38400:
					cfsetspeed(&param, B38400);
					break;
				case 57600:
					cfsetspeed(&param, B57600);
					break;
				case 115200:
					cfsetspeed(&param, B115200);
					break;
				default:
					cfsetspeed(&param, B9600);
					break;
			}
			// Set byte size
			param.c_cflag &= ~CSIZE;
			switch (bits)
			{
				case 5:
					param.c_cflag |= CS5;
					break;
				case 6:
					param.c_cflag |= CS6;
					break;
				case 7:
					param.c_cflag |= CS7;
					break;
				case 8:
					param.c_cflag |= CS8;
					break;
				default:
					param.c_cflag |= CS8;
					break;
			}
			// Set parity
			switch (parity)
			{
				case Parity::NO:
					param.c_cflag &= ~PARENB; // Disable parity
					break;
				case Parity::EVEN:
					param.c_cflag |= PARENB;  // Enable parity
					param.c_cflag &= ~PARODD; // Disable odd parity
					break;
				case Parity::ODD:
					param.c_cflag |= PARENB;  // Enable parity
					param.c_cflag |= PARODD;  // Enable odd parity
					break;
				default:
					param.c_cflag &= ~PARENB; // Disable parity
					break;
			}
			// Set stop bit
			if (stopbit)
				param.c_cflag |= CSTOPB;       // Enable 2 stop bits
			else
				param.c_cflag &= ~CSTOPB;	   // Disable 2 stop bits
			// Enable receiver (CREAD) and ignore modem control lines (CLOCAL)
			param.c_cflag |= (CREAD | CLOCAL);
			// Disable, canonical mode (ICANON = 0), echo input character (ECHO) and signal generation (ISIG)
			param.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			// Disable input parity checking (INPCK)
			param.c_iflag &= ~INPCK;
			// Disable XON/XOFF flow control on output and input (IXON/IXOFF), framing and parity errors (IGNPAR), and disable CR to NL translation
			param.c_iflag &= ~(IXON | IXOFF | IXANY | IGNPAR | ICRNL);
			// Disable implementation-defined output processing (OPOST)
			param.c_oflag &= ~OPOST;
			// Set terminal parameters
			tcsetattr(_fd, TCSAFLUSH, &param);
			return true;
		}

		void serial_close()
		{
			if (_fd > 0)
				close(_fd);
		}

		~Serial()
		{
			serial_close();
		}

		bool ok() const
		{
			return (_fd >= 0);
		}

		// Пишем данные.
		// buf - данные.
		// size - размер данных.
		// Возвращает true, если успех.
		bool write_data(const uint8_t* buf, size_t size)
		{
			if (_fd < 0)
				return false;
			// Send data
			while (size > 0)
			{
				ssize_t r = write(_fd, buf, size);
				if (r < 1)
					return false;
				size -= r;
				buf += r;
			}
			return true;
		}

		bool write_data(const std::string& data)
		{
			return write_data((const uint8_t*)data.c_str(), data.size());
		}

		// Читаем данные из порта.
		// buf - куда читаем.
		// size - размер буфера.
		// Возвращает количество прочитанных байт.
		size_t read_data(uint8_t* buf, size_t size)
		{
			if (_fd < 0)
				return 0;
			size_t len = 0;
			while (true)
			{
				ssize_t r = read(_fd, (buf + len), size - len);
				if (r <= 0)
					break;
				len += r;
				if (len >= size)
					break;
			}
			return len;
		}
	};
}