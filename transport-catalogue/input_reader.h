#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"
namespace transport_catalogue {
    namespace input {
        struct CommandDescription {
            // Определяет, задана ли команда (поле command непустое)
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;      // Название команды
            std::string id;           // id маршрута или остановки
            std::string description;  // Параметры команды
        };

        class InputReader {
        public:            
            void Load(std::istream& in, TransportCatalogue& catalogue);
        private:
            void ParseLine(std::string_view line);
            void ApplyCommands(TransportCatalogue& catalogue) const;

            std::vector<CommandDescription> commands_;
        };

        CommandDescription ParseCommandDescription(std::string_view line);
        std::vector<std::string_view> ParseRoute(std::string_view route);
        std::vector<std::pair<std::string_view, double>> ParseDistance(std::string_view str);
    }
}

