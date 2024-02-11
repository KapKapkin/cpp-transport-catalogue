#include <unordered_set>
#include <stdexcept>
#include <vector>

#include "transport_catalogue.h"
#include "string_view"

namespace transport_catalogue {
	void TransportCatalogue::AddBus(const std::string& busname) {
		buses_.push_back(std::move(Bus(busname)));
		busname_to_bus_[buses_.back().name_] = &buses_.back();
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

	size_t TransportCatalogue::GetUniqueStopsNum(std::string_view busname) const {
		const Bus* bus_p = busname_to_bus_.at(busname);
		std::unordered_set<std::string_view> count;
		for (auto stop : bus_p->stops_) {
			count.insert(stop->name_);
		}
		return count.size();
	}

	bool TransportCatalogue::CompareBusesPtrs:: operator() (std::string_view lhs, std::string_view rhs) const {
		return std::lexicographical_compare(lhs.begin(), lhs.end(),
			rhs.begin(), rhs.end());
	}
}

