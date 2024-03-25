#pragma once

#include <algorithm>
#include <string>
#include <deque>
#include <unordered_map>
#include <utility>
#include <vector>
#include <set>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		TransportCatalogue(const TransportCatalogue&) = delete;
		TransportCatalogue& operator=(const TransportCatalogue&) = delete;

		void AddBus(const std::string& busname);
		void SetBusRouteType(std::string_view busname, RouteType type);

		const Bus& GetBusByName(std::string_view busname) const;

		double GetRouteLength(std::string_view busname) const;
		double GetGeoRouteLength(std::string_view busname) const;
		double GetRouteCurvature(std::string_view busname) const;
		size_t GetStopCount(std::string_view busname) const;
		size_t GetUniqueStopsCount(std::string_view busname) const;

		void AddStop(const std::string& stopname, geo::Coordinates coords);
		const Stop& GetStopByName(std::string_view stopname) const;
		std::vector<Bus*> GetBuses() ;

		void AddStopForBus(std::string_view busname, std::string_view stopname);
		std::vector<std::string_view> GetBusnamesForStop(std::string_view stopname) const;

		

		void SetDistance(std::string_view stopname1, std::string_view stopname2, double distance);
		int GetDistance(std::string_view stopname1, std::string_view stopname2) const;

	private:

		struct StopPairHash {
			size_t operator() (std::pair<Stop*, Stop*> stop_pair) const {
				static const int x = 43;
				return std::hash<std::string>{}(stop_pair.first->name_) + std::hash<std::string>{}(stop_pair.second->name_)* x;
			}
		};

		std::deque<Bus> buses_;
		std::deque<Stop> stops_;

		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;

		std::unordered_map<std::pair<Stop*, Stop*>, int, StopPairHash> stops_to_distance_;

		std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_busnames_;
	};

	namespace detail {
		std::string_view Trim(std::string_view string);
		std::vector<std::string_view> Split(std::string_view string, char delim);
	}
}