#include <unordered_set>
#include <stdexcept>
#include <vector>

#include "transport_catalogue.h"
#include "string_view"
#include "geo.h"

namespace transport_catalogue {
	void TransportCatalogue::AddBus(const std::string& busname) {
		buses_.push_back(std::move(Bus(busname)));
		busname_to_bus_[buses_.back().name_] = &buses_.back();
	}

	void TransportCatalogue::SetBusRouteType(std::string_view busname, RouteType type) {
		busname_to_bus_[busname]->route_type_ = type;
	}

	const Bus& TransportCatalogue::GetBusByName(std::string_view busname) const {
		if (busname_to_bus_.count(busname) == 0) {
			throw std::invalid_argument("");
		}
		return *busname_to_bus_.at(busname);
	}

	void TransportCatalogue::AddStop(const std::string& stopname, geo::Coordinates coords) {
		stops_.push_back({ stopname, coords });
		stopname_to_stop_[stops_.back().name_] = &stops_.back();
		stop_to_busnames_[stops_.back().name_];
	}

	const Stop& TransportCatalogue::GetStopByName(std::string_view stopname) const {
		return *stopname_to_stop_.at(stopname);
	}

	std::vector<Bus*> TransportCatalogue::GetBuses() {
		std::vector<Bus*> buses;
		for (auto& bus : buses_) {
			buses.push_back(&bus);
		}
		return buses;
	}

	void TransportCatalogue::AddStopForBus(std::string_view busname, std::string_view stopname) {
		Bus* bus = busname_to_bus_.at(busname);
		Stop* stop = stopname_to_stop_.at(stopname);

		bus->stops_.push_back(stop);

		stop_to_busnames_[stopname].insert(bus->name_);
	}

	std::vector<std::string_view> TransportCatalogue::GetBusnamesForStop(std::string_view stopname) const {
		std::vector<std::string_view> res;
		for (auto bus : stop_to_busnames_.at(stopname)) {
			res.push_back(bus);
		}
		return res;
	}


	double TransportCatalogue::GetRouteLength(std::string_view busname) const {
		double result = 0;
		const auto* bus = busname_to_bus_.at(busname);

		for (size_t i = 0; i < bus->stops_.size() - 1; i++) {
			result += GetDistance(bus->stops_[i]->name_, bus->stops_[i + 1]->name_);
		}
		if (bus->route_type_ == RouteType::DIRECT) {
			for (size_t i = bus->stops_.size() - 1; i > 0; i--) {
				result += GetDistance(bus->stops_[i]->name_, bus->stops_[i - 1]->name_);
			}
		}
		
		return result;
	}

	double TransportCatalogue::GetGeoRouteLength(std::string_view busname) const {
		double result = 0;
		const auto* bus = busname_to_bus_.at(busname);

		for (size_t i = 0; i < bus->stops_.size() - 1; i++) {
			result += geo::ComputeDistance(bus->stops_[i]->coords_, bus->stops_[i + 1]->coords_);
		}
		if (bus->route_type_ == RouteType::DIRECT) {
			for (size_t i = bus->stops_.size() - 1; i > 0; i--) {
				result += geo::ComputeDistance(bus->stops_[i]->coords_, bus->stops_[i - 1]->coords_);
			}
		}
		return result;
	}

	double TransportCatalogue::GetRouteCurvature(std::string_view busname) const {
		double fact_length = GetRouteLength(busname);
		double geo_length = GetGeoRouteLength(busname);
		return (fact_length / geo_length);
	}

	size_t TransportCatalogue::GetStopCount(std::string_view busname) const {
		auto* bus = busname_to_bus_.at(busname);
		size_t res = bus->stops_.size();
		if (bus->route_type_ == RouteType::DIRECT) {
			res = res * 2 - 1;
		}
		return res;
	}

	size_t TransportCatalogue::GetUniqueStopsCount(std::string_view busname) const {
		const Bus* bus_p = busname_to_bus_.at(busname);
		std::unordered_set<std::string_view> count;
		for (auto stop : bus_p->stops_) {
			count.insert(stop->name_);
		}
		return count.size();
	}

	void TransportCatalogue::SetDistance(std::string_view stopname1, std::string_view stopname2, double distance) {
		Stop* bus1 = stopname_to_stop_[stopname1];
		Stop* bus2 = stopname_to_stop_[stopname2];
		if (stops_to_distance_.count({ bus1, bus2 }) == 1) {
			stops_to_distance_[{bus1, bus2}] = distance;
		}
		else {
			stops_to_distance_[{bus1, bus2}] = distance;
			stops_to_distance_[{bus2, bus1}] = distance;
		}
	}

	int TransportCatalogue::GetDistance(std::string_view stopname1, std::string_view stopname2) const {
		Stop* bus1 = stopname_to_stop_.at(stopname1);
		Stop* bus2 = stopname_to_stop_.at(stopname2);
		return stops_to_distance_.at({ bus1, bus2 });
	}

	namespace detail {

		std::string_view Trim(std::string_view string) {
			const auto start = string.find_first_not_of(' ');
			if (start == string.npos) {
				return {};
			}
			return string.substr(start, string.find_last_not_of(' ') + 1 - start);
		}


		std::vector<std::string_view> Split(std::string_view string, char delim) {
			std::vector<std::string_view> result;

			size_t pos = 0;
			while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
				auto delim_pos = string.find(delim, pos);
				if (delim_pos == string.npos) {
					delim_pos = string.size();
				}
				if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
					result.push_back(substr);
				}
				pos = delim_pos + 1;
			}

			return result;
		}
	}

}