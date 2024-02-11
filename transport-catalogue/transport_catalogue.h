#pragma once

#include <algorithm>
#include <string>
#include <deque>
#include <unordered_map>
#include <utility>
#include <vector>
#include <set>

#include "geo.h"

namespace transport_catalogue {

	struct Stop {

		Stop() = default;
		Stop(const std::string& name) : name_(name) {}
		Stop(const std::string& name, const geo::Coordinates& coords) : name_(name), coords_(coords) {}

		std::string name_;
		geo::Coordinates coords_;
	};

	struct Bus {

		Bus() = default;
		Bus(const std::string& name) :name_(name) {}

		std::string name_;
		std::vector<Stop*> stops_;
	};

	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		TransportCatalogue(const TransportCatalogue&) = delete;
		TransportCatalogue& operator=(const TransportCatalogue&) = delete;

		void AddBus(const std::string& busname);
		const Bus& GetBusByName(std::string_view busname) const;

		void AddStop(const std::string& stopname, geo::Coordinates coords);
		const Stop& GetStopByName(std::string_view stopname) const;

		void AddStopForBus(std::string_view busname, std::string_view stopname);
		std::vector<std::string_view> GetBusnamesForStop(std::string_view stopname) const;

		size_t GetUniqueStopsNum(std::string_view busname) const;


	private:
		struct CompareBusesPtrs {
			bool operator() (std::string_view lhs, std::string_view rhs) const;
		};

		std::deque<Bus> buses_;
		std::deque<Stop> stops_;

		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;

		std::unordered_map<std::string_view, std::set<std::string_view, CompareBusesPtrs>> stop_to_busnames_;
	};
}

