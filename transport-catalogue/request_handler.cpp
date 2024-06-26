#include "request_handler.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>


#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_catalogue.h"
#include "transport_router.h"


using namespace transport_catalogue;
using namespace map_renderer;
using namespace std::literals;

namespace transport_catalogue {

	namespace detail {
		namespace render_settings {
			MapRenderSettings GetRenderSettings(std::unordered_map<std::string_view, const json::Node*> data) {

				MapRenderSettings settings;

				settings.width = data.at("width")->AsDouble();
				settings.height = data.at("height")->AsDouble();
				settings.padding = data.at("padding")->AsDouble();
				settings.line_width = data.at("line_width")->AsDouble();
				settings.stop_radius = data.at("stop_radius")->AsDouble();
				settings.bus_label_font_size = data.at("bus_label_font_size")->AsInt();
				settings.bus_label_offset = std::move(ParseOffset(data.at("bus_label_offset")->AsArray()));
				settings.stop_label_font_size = data.at("stop_label_font_size")->AsInt();
				settings.stop_label_offset = std::move(ParseOffset(data.at("stop_label_offset")->AsArray()));
				settings.underlayer_color = std::move(ParseColor(data.at("underlayer_color")));
				settings.underlayer_width = data.at("underlayer_width")->AsDouble();
				settings.color_palette = std::move(ParsePaletteColors(data.at("color_palette")->AsArray()));
				return settings;
			}

			svg::Color ParseColor(const json::Node* data) {
				if (data->IsString()) {
					return data->AsString();
				}
				else if (data->IsArray()) {
					if (data->AsArray().size() == 3) {
						svg::Rgb rgb_color = { (uint8_t)data->AsArray()[0].AsDouble(), (uint8_t)data->AsArray()[1].AsDouble(), (uint8_t)data->AsArray()[2].AsDouble() };
						return rgb_color;
					}
					else if (data->AsArray().size() == 4) {
						svg::Rgba rgba_color = { (uint8_t)data->AsArray()[0].AsDouble(), (uint8_t)data->AsArray()[1].AsDouble(), (uint8_t)data->AsArray()[2].AsDouble(), data->AsArray()[3].AsDouble() };
						return rgba_color;
					}
					else {
						throw json::ParsingError("");
					}
				}
				else {
					throw json::ParsingError("");
				}
			}

			std::vector<svg::Color> ParsePaletteColors(const json::Array& data) {
				std::vector<svg::Color> colors;
				for (const json::Node& color : data) {
					colors.push_back(std::move(ParseColor(&color)));
				}
				return colors;
			}

			svg::Point ParseOffset(const json::Array& data) {
				if (data.size() == 2) {
					return { data[0].AsDouble(), data[1].AsDouble() };
				}
				else {
					throw json::ParsingError("");
				}
			}
		}// --------------- render_settings ----------------

	} // ------------------ detail -------------------------

	namespace requests {

		using namespace detail::render_settings;
		using namespace domain;

		// ---------- JSONHandler -----------------

		RequestHandler::RequestHandler(TransportCatalogue& catalogue, std::istream& in) : db_(catalogue), reader_(in) {
			ApplyRequest();
		}

		void RequestHandler::ApplyRequest() {
			ApplyStopRequests();
			ApplyBusRequests();
			ApplyRoutingSettings();
		}

		void RequestHandler::Router() {
			graph_ = std::make_unique<transport_graph::TransportGraph>(transport_graph::TransportGraph(db_));
			router_ = std::make_unique<transport_graph::TransportRouter>(transport_graph::TransportRouter(*graph_));
		}

		void RequestHandler::ApplyStopRequests() {
			std::vector<const json::Node*> stop_requests = reader_.GetStopRequests();
			for (const auto* request : stop_requests) {
				const json::Dict& stop_info = request->AsDict();
				db_.AddStop(stop_info.at("name").AsString(), { stop_info.at("latitude").AsDouble(),  stop_info.at("longitude").AsDouble() });
			}

			std::unordered_map<std::string_view, const json::Dict*> road_distances = reader_.GetRoadDistances();
			for (const auto& [name1, distances] : road_distances) {
				for (const auto& [name2, distance] : *distances) {
					db_.SetDistance(name1, name2, distance.AsDouble());
				}
			}
		}

		void RequestHandler::ApplyBusRequests() {
			std::vector<const json::Node*> bus_requests = reader_.GetBusRequests();
			for (const auto* request : bus_requests) {
				const json::Dict& bus_info = request->AsDict();
				const std::string& busname = bus_info.at("name").AsString();

				db_.AddBus(busname);
				db_.SetBusRouteType(busname, IntToRouteType((int)bus_info.at("is_roundtrip").AsBool()));
				const json::Array& stops = bus_info.at("stops").AsArray();
				for (size_t i = 0; i < stops.size(); i++) {
					db_.AddStopForBus(busname, stops[i].AsString());
				}
			}
		}

		void RequestHandler::ApplyRoutingSettings() {
			const auto& settings = reader_.GetRoutingSettings();
			db_.SetRoutingSettings({ settings.at("bus_wait_time"), settings.at("bus_velocity") });
		}

		void RequestHandler::ExecuteStatRequest(std::ostream& out) {
			const std::vector<const json::Node*>& stat_requests = reader_.GetStatRequests();

			json::Builder builder;
			builder.StartArray();

			for (const json::Node* requests : stat_requests) {

				const json::Dict& request_data = requests->AsDict();
				const std::string& request_type = request_data.at("type").AsString();

				if (request_type == "Stop") {
					ApplySingleStopRequest(builder, request_data);
				}
				else if (request_type == "Bus") {
					ApplySingleBusRequest(builder, request_data);
				}
				else if (request_type == "Map") {
					ApplySingleMapRequest(builder, request_data);
				}
				else if (request_type == "Route") {
					ApplySingleRouteRequest(builder, request_data);
				}
			}

			builder.EndArray();
			json::Print(json::Document(builder.Build()), out);
		}

		void RequestHandler::ApplySingleStopRequest(json::Builder& builder, const json::Dict& request_data) {
			builder.StartDict().Key("request_id").Value(request_data.at("id").AsInt());
			const std::string name = request_data.at("name").AsString();

			try {
				json::Array buses;
				for (std::string_view busname : db_.GetBusnamesForStop(name)) {
					buses.push_back((std::string)busname);
				}
				builder.Key("buses").Value(json::Node(buses));
			}
			catch (const std::out_of_range&) {
				builder.Key("error_message").Value("not found");
			}

			builder.EndDict();
		}

		void RequestHandler::ApplySingleBusRequest(json::Builder& builder, const json::Dict& request_data) {
			builder.StartDict().Key("request_id").Value(request_data.at("id").AsInt());
			const std::string name = request_data.at("name").AsString();

			try {
				double curvature = db_.GetRouteCurvature(name);
				double route_length = db_.GetRouteLength(name);
				int stop_count = db_.GetStopCount(name);
				int unique_stop_count = db_.GetUniqueStopsCount(name);

				builder.Key("curvature").Value(curvature);
				builder.Key("route_length").Value(route_length);
				builder.Key("stop_count").Value(stop_count);
				builder.Key("unique_stop_count").Value(unique_stop_count);
			}
			catch (const std::out_of_range&) {
				builder.Key("error_message").Value("not found");
			}

			builder.EndDict();
		}

		void RequestHandler::ApplySingleMapRequest(json::Builder& builder, const json::Dict& request_data) {
			builder.StartDict().Key("request_id").Value(request_data.at("id").AsInt());

			if (!GetMap().has_value()) {
				throw std::logic_error("");
			}

			builder.Key("map").Value(std::move(*GetMap()));
			builder.EndDict();
		}

		void RequestHandler::ApplySingleRouteRequest(json::Builder& builder, const json::Dict& request_data) {
			if (router_ == nullptr) {
				throw std::logic_error("");
			}
			builder.StartDict().Key("request_id").Value(request_data.at("id").AsInt());

			const Stop* from = db_.GetStopByName( request_data.at("from").AsString());
			const Stop* to = db_.GetStopByName(request_data.at("to").AsString());
			auto route_data = router_->GetRoute(from, to);
			if (route_data.has_value()) {
				builder.Key("items").StartArray();
				for (const auto& data : route_data.value().route) {
					builder.StartDict();
					if (data.bus == nullptr) {
						builder.Key("stop_name").Value(data.stop_from->name_);
						builder.Key("time").Value(data.time);
						builder.Key("type").Value("Wait");
					}
					else {
						builder.Key("bus").Value(data.bus->name_);
						builder.Key("span_count").Value(data.stop_count);
						builder.Key("time").Value(data.time);
						builder.Key("type").Value("Bus");
					}
					builder.EndDict();
				}
				builder.EndArray();
				builder.Key("total_time").Value(route_data.value().time);
			}
			else {
				builder.Key("error_message").Value("not found");
			}
			builder.EndDict();
		}
			
		void RequestHandler::Render() {
			MapRenderSettings settings = GetRenderSettings(reader_.GetRenderSettings());
			std::vector<Bus*> buses;
			for (auto it : db_.GetBuses()) {
				buses.push_back(it.second);
			}
			MapRenderer renderer(std::move(settings), buses);
			std::ostringstream oss;
			renderer.Render(oss);

			rendered_map_ = oss.str();
		}

		std::optional<std::string> RequestHandler::GetMap() const {
			return rendered_map_;
		}

	} // namespace requests

} // namespace transport_catalogue



