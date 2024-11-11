// Copyright (c) 2024 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <errno.h>
#include <iostream>
#include <string.h>


namespace app
{
	namespace _
	{
		const char* color_red = "\033[1;31m";
		const char* color_gray = "\033[1;90m";
		const char* color_end = "\033[0m";
	}

	bool print_errno(const char* const msg)
	{
		std::cout << _::color_red << "Error! " << msg << ": " << strerror(errno) << " (" << errno << ")." << _::color_end << std::endl;
		return false;
	}

	bool print_error(const char* const msg)
	{
		std::cout << _::color_red << "Error! " << msg << _::color_end << std::endl;
		return false;
	}

	bool print_error(const char* const msg, const char* const arg)
	{
		std::cout << _::color_red << "Error! " << msg << arg << _::color_end << std::endl;
		return false;
	}

	void print_notice(const char* const msg)
	{
		std::cout << _::color_gray << msg << _::color_end << std::endl;
	}

	void print_notice(const char* const msg, const char* const arg)
	{
		std::cout << _::color_gray << msg << arg << _::color_end << std::endl;
	}
}