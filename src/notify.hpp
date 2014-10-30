#pragma once

#include <boost/asio.hpp>
#include "dir_monitor.hpp"

namespace notify
{
	class manager
	{
	public:
		manager();
		~manager();
		void poll();
	};

	void register_notification_path(const std::string& path, 
		boost::function<void(const std::string&, const boost::asio::dir_monitor_event&)> fn);
}
