#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

	enum class RouteType {
		DIRECT,
		ROUND,
		UNKNOWN
		};

	RouteType IntToRouteType(int i);

		struct Stop {

			Stop() = default;
			Stop(const std::string& name) : name_(name) {}
			Stop(const std::string& name, const geo::Coordinates& coords) : name_(name), coords_(coords) {}

			std::string name_;
			geo::Coordinates coords_{};
		};


		struct Bus {

			Bus() = default;
			Bus(const std::string& name) :name_(name) {}

			std::string name_;
			std::vector<Stop*> stops_;
			RouteType route_type_ = RouteType::ROUND;
		};
}