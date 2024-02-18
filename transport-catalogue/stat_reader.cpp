#include <iostream>
#include <string>
#include <string_view>

#include "stat_reader.h"
#include "geo.h"

namespace transport_catalogue {
    namespace statisctics {

        void ParseInputAndPrintStat(std::istream& in, std::ostream& out, TransportCatalogue& catalogue) {
            int stat_request_count;
            in >> stat_request_count >> std::ws;
            for (int i = 0; i < stat_request_count; ++i) {
                std::string line;
                getline(in, line);
                statisctics::ParseAndPrintStat(catalogue, line, out);
            }
        }

        static std::pair<std::string_view, std::string_view> ParseRequest(std::string_view request) {
            auto pos = request.find_first_of(' ');
            return { detail::Trim(request.substr(0, pos)), detail::Trim(request.substr(pos + 1, request.npos)) };
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

                double fact_length = 0;
                double geo_length = 0;
                for (size_t i = 0; i < bus.stops_.size() - 1; i++) {
                    fact_length += transport_catalogue.GetDistance(bus.stops_[i]->name_, bus.stops_[i + 1]->name_);
                    geo_length += geo::ComputeDistance(bus.stops_[i]->coords_, bus.stops_[i + 1]->coords_);
                }
                output << bus.stops_.size() << " stops on route, " << transport_catalogue.GetUniqueStopsNum(id) << " unique stops, " << fact_length << " route length, " << fact_length / geo_length << " curvature" << std::endl;
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

