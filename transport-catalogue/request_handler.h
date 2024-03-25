#pragma once

#include <optional>
#include <unordered_set>

#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "domain.h"
#include "map_renderer.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace transport_catalogue {

    namespace detail {
        map_renderer::MapRenderSettings GetRenderSettings(std::unordered_map<std::string_view, const json::Node*> data);
        svg::Color ParseColor(const json::Node* data);
        std::vector<svg::Color> ParsePaletteColors(const json::Array& data);
        svg::Point ParseOffset(const json::Array& data);
    }
    namespace requests {

        class RequestHandler {
        public:
            RequestHandler(TransportCatalogue& catalogue, std::istream& in);

            void ExecuteStatRequest(std::ostream& out);

            void Render();
            
            std::string GetMap() {
                return rendered_map_;
            }

            //std::optional<Bus*> GetBusStat(const std::string_view& bus_name) const;

            //const std::unordered_set<Bus*>* GetBusesByStop(const std::string_view& stop_name) ;
                
        private:

            void ApplyRequest();

            void ApplyStopRequests();
            void ApplyBusRequests();
           
            TransportCatalogue& db_;
            const json_reader::JSONReader reader_;
            std::string rendered_map_;
        };
    }

}


