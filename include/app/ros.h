// https://github.com/IOdissey/app
// Copyright (c) 2025 Alexander Abramenkov. All rights reserved.
// Distributed under the MIT License (license terms are at https://opensource.org/licenses/MIT).

#pragma once

#include <memory>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <ros/ros.h>
#include <geometry_msgs/Vector3.h>
#include <sensor_msgs/Imu.h>
#pragma GCC diagnostic pop


namespace app::ros_app
{
	namespace _
	{
		bool is_init = false;
		std::shared_ptr<::ros::NodeHandle> node;
	}

	bool init()
	{
		if (_::is_init)
			return true;
		int argc = 0;
		char **argv = nullptr;
		ros::init(argc, argv, "ros_app", ros::init_options::NoSigintHandler | ros::init_options::AnonymousName);
		ros::Time::init();
		if (!ros::master::check())
		{
			std::cout << "ROS master not started." << std::endl;
			return false;
		}
		_::node = std::make_shared<::ros::NodeHandle>("~");
		_::is_init = true;
		return true;
	}

	void update()
	{
		if (_::is_init)
			ros::spinOnce();
	}

	std::shared_ptr<::ros::NodeHandle> node()
	{
		return _::node;
	}
}