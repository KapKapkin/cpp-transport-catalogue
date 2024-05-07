#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

#include "domain.h"
#include "json_builder.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_catalogue.h"
#include "transport_router.h"


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

            void Router();
            void Render();
            
            std::optional<std::string> GetMap() const;
                
        private:

            void ApplyRequest();

            void ApplyStopRequests();
            void ApplyBusRequests();
            void ApplyRoutingSettings();
            
            void ApplySingleBusRequest(json::Builder& builder, const json::Dict& request_data);
            void ApplySingleStopRequest(json::Builder& builder, const json::Dict& request_data);
            void ApplySingleMapRequest(json::Builder& builder, const json::Dict& request_data);
            void ApplySingleRouteRequest(json::Builder& builder, const json::Dict& request_data);
           
            TransportCatalogue& db_;
            const json_reader::JSONReader reader_;
            std::optional<std::string> rendered_map_;

            std::unique_ptr<transport_graph::TransportGraph> graph_;
            std::unique_ptr<transport_graph::TransportRouter> router_;
        };
    } // ------------------ namespace requests ----------------

} // ------------------ namespace transport_catalogue ----------------



