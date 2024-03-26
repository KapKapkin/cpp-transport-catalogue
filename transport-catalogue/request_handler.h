#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "domain.h"
#include "map_renderer.h"

namespace transport_catalogue {

    namespace detail {
        namespace render_settings {
            map_renderer::MapRenderSettings GetRenderSettings(std::unordered_map<std::string_view, const json::Node*> data);
            svg::Color ParseColor(const json::Node* data);
            svg::Point ParseOffset(const json::Array& data);
            std::vector<svg::Color> ParsePaletteColors(const json::Array& data);
        }// --------------- render_settings ----------------
    } // ------------------ detail ----------------
    namespace requests {

        class RequestHandler {
        public:
            RequestHandler(TransportCatalogue& catalogue, std::istream& in);

            void ExecuteStatRequest(std::ostream& out);

            void Render();
            
            std::string GetMap() {
                return rendered_map_;
            }
                
        private:

            void ApplyRequest();

            void ApplyStopRequests();
            void ApplyBusRequests();
           
            TransportCatalogue& db_;
            const json_reader::JSONReader reader_;
            std::string rendered_map_;
        };
    } // ------------------ namespace requests ----------------

} // ------------------ namespace transport_catalogue ----------------



