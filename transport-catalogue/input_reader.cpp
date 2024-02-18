#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <string_view>
#include <string>


namespace transport_catalogue {
    namespace input {
        static geo::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return { lat, lng };
        }

        /**
         * Удаляет пробелы в начале и конце строки
         */
        

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return detail::Split(route, '>');
            }
            auto stops = detail::Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return { std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1)) };
        }

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        std::vector<std::pair<std::string_view, double>> ParseDistance(std::string_view str) {
            std::vector<std::pair<std::string_view, double>> res;
            std::vector<std::string_view> splited_info = detail::Split(str, ',');

            if (splited_info.size() <= 2) {
                return res;
            }

            for (auto it = splited_info.begin() + 2; it != splited_info.end(); it++) {
                auto not_space1 = it->find_first_not_of(' ');
                auto not_space2 = it->find_first_not_of(' ', it->find_first_of(' ', not_space1));
                auto not_space3 = it->find_first_not_of(' ', it->find_first_of(' ', not_space2));

                double distance = std::stod(std::string(it->substr(not_space1)));
                std::string_view stop = it->substr(not_space3);
                res.push_back({ stop, distance });
            }

            return res;
        }


        void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
            for (const auto& command : commands_) {
                if (command.command == "Stop") {
                    catalogue.AddStop(command.id, ParseCoordinates(command.description));
                }
            }
            for (const auto& command : commands_) {
                if (command.command == "Bus") {
                    catalogue.AddBus(command.id);
                    auto stops = ParseRoute(command.description);
                    for (const auto& stop : stops) {
                        catalogue.AddStopForBus(command.id, stop);
                    }
                }
            }

            for (const auto& command : commands_) {
                if (command.command == "Stop") {
                    auto distance_info = ParseDistance(command.description);
                    for (const auto& info : distance_info) {
                        catalogue.SetDistance(command.id, info.first, info.second);
                    }
                }
            }
        }

        void InputReader::Load(std::istream& in, TransportCatalogue& catalogue) {
            int base_request_count;
            in >> base_request_count >> std::ws;
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                getline(in, line);
                ParseLine(line);
            }
            ApplyCommands(catalogue);
        }



    }
}

