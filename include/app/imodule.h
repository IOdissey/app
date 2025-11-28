// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include "app/config.h"
#include "json.h"


namespace app
{
	template <typename T>
	class IModule
	{
	public:
		virtual ~IModule()
		{
		}

		virtual bool beg(const app::Config&)
		{
			return true;
		}

		virtual void update(T&, double)
		{
		};

		virtual void param(app::Json&, T&)
		{
		};

		virtual void end()
		{
		}
	};
}