#include <iostream>
#include <string_view>

#include "stat_reader.h"
#include "geo.h"

namespace transport_catalogue {
    namespace statisctics {
        static std::pair<std::string_view, std::string_view> ParseRequest(std::string_view request) {
            auto pos = request.find_first_of(' ');
            return { request.substr(0, pos), request.substr(pos + 1, request.npos) };
        }

        void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
            std::ostream& output) {
            output << request << ':' << ' ';
            auto [command, id] = ParseRequest(request);
            if (command == "Bus") {
                Bus bus;
                try {
                    bus = transport_catalogue.GetBusByName(id);
                }
                catch (...) {
                    output << "not found" << std::endl;
                    return;
                }
                double length = 0;
                for (size_t i = 0; i < bus.stops_.size() - 1; i++) {
                    length += geo::ComputeDistance(bus.stops_[i]->coords_, bus.stops_[i + 1]->coords_);
                }
                output << bus.stops_.size() << " stops on route, " << transport_catalogue.GetUniqueStopsNum(id) << " unique stops, " << length << " route length" << std::endl;
            }
            else if (command == "Stop") {
                std::vector<std::string_view> buses;
                try {
                    buses = transport_catalogue.GetBusnamesForStop(id);
                }
                catch (...) {
                    output << "not found" << std::endl;
                    return;
                }
                if (buses.empty()) {
                    output << "no buses" << std::endl;
                    return;
                }
                output << "buses";
                for (auto busname : buses) {
                    output << ' ' << busname;
                }
                output << std::endl;
            }
        }
    }
   

}

