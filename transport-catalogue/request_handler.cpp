#include <iostream>

#include "request_handler.h"
#include "map_renderer.h"
#include "json.h"
#include "sstream"


/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
using namespace transport_catalogue;
using namespace map_renderer;
using namespace std::literals;

namespace transport_catalogue {

	namespace detail {
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

	} // --------------- detail ----------------

	namespace requests {

		// ---------- JSONHandler -----------------

		RequestHandler::RequestHandler(TransportCatalogue& catalogue, std::istream& in) : db_(catalogue), reader_(in) {
			ApplyRequest();
		}

		void RequestHandler::ApplyRequest() {
			ApplyStopRequests();
			ApplyBusRequests();
		}

		void RequestHandler::ApplyStopRequests() {
			std::vector<const json::Node*> stop_requests = reader_.GetStopRequests();
			for (const auto* request : stop_requests) {
				const json::Dict& stop_info = request->AsMap();
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
				const json::Dict& bus_info = request->AsMap();
				const std::string& busname = bus_info.at("name").AsString();

				db_.AddBus(busname);
				db_.SetBusRouteType(busname, IntToRouteType((int)bus_info.at("is_roundtrip").AsBool()));
				const json::Array& stops = bus_info.at("stops").AsArray();
				for (size_t i = 0; i < stops.size(); i++) {
					db_.AddStopForBus(busname, stops[i].AsString());
				}
			}
		}

		void RequestHandler::ExecuteStatRequest(std::ostream& out) {
			const std::vector<const json::Node*>& stat_requests = reader_.GetStatRequests();
			json::Array res;

			for (const json::Node* requests : stat_requests) {
				const json::Dict& request_data = requests->AsMap();
				

				json::Dict response;
				response["request_id"] = request_data.at("id").AsInt();

				const std::string& request_type = request_data.at("type").AsString();

				if (request_type == "Stop") {
					const std::string& name = request_data.at("name").AsString();

					try {
						json::Array buses;
						for (std::string_view busname : db_.GetBusnamesForStop(name)) {
							buses.push_back((std::string)busname);
						}
						response["buses"] = json::Node(buses);
					}
					catch (const std::out_of_range&) {
						response["error_message"] = "not found";
					}
				}
				else if (request_type == "Bus") {
					const std::string& name = request_data.at("name").AsString();

					try {
						response["curvature"] = db_.GetRouteCurvature(name);
						response["route_length"] = db_.GetRouteLength(name);
						response["stop_count"] = (int)db_.GetStopCount(name);
						response["unique_stop_count"] = (int)db_.GetUniqueStopsCount(name);
					}
					catch (const std::out_of_range&) {
						response["error_message"] = "not found";
					}
				}
				else if (request_type == "Map") {
					if (!GetMap().size() ) {
						throw std::logic_error("");
					}
					response["map"] = std::move(GetMap());
				}
				res.push_back(response);
			}
			json::Print(json::Document(res), out);
		}
			
		void RequestHandler::Render() {
			MapRenderSettings settings = detail::GetRenderSettings(reader_.GetRenderSettings());
			MapRenderer renderer(std::move(settings), db_.GetBuses());

			std::ostringstream oss;
			renderer.Render(oss);

			rendered_map_ = oss.str();
		}

	} // namespace requests
} // namespace transport_catalogue


