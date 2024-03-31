#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "json.h"
#include "transport_catalogue.h"

#include "json_reader.h"


using namespace std::literals;

namespace transport_catalogue {
	namespace json_reader {

		// -------------- JSONReader -------------- 


		JSONReader::JSONReader(std::istream& in) :commands_(json::Load(in)) {
			FillBusRequests();
			FillStopRequests();
			FillStatRequests();
			FillRenderSettings();
		}

		const std::vector<const json::Node*>& JSONReader::GetStatRequests() const {
			return stat_requests_;
		}

		const std::vector<const json::Node*>& JSONReader::GetBusRequests() const {
			return bus_requests_;
		}

		const std::vector<const json::Node*>&  JSONReader::GetStopRequests() const {
			return stop_requests_;
		}

		const std::unordered_map<std::string_view, const json::Dict*>& JSONReader::GetRoadDistances() const {
			return road_distances_;
		}

		const std::unordered_map<std::string_view, const json::Node*>& JSONReader::GetRenderSettings() const {
			return render_settings_;
		}



		void JSONReader::FillBusRequests() {

			const auto& root = commands_.GetRoot();
			const json::Array& base_requests = root.AsDict().at("base_requests").AsArray();

			for (const auto& request : base_requests) {
				const json::Dict& request_data = request.AsDict();

				if (request_data.at("type").AsString() == "Bus") {

					bus_requests_.push_back(&request);

				}
			}
		}

		void JSONReader::FillStopRequests() {

			auto& root = commands_.GetRoot();
			const json::Array& base_requests = root.AsDict().at("base_requests").AsArray();

			for (const json::Node& request : base_requests) {
				const json::Dict& request_data = request.AsDict();

				if (request_data.at("type").AsString() == "Stop") {
					stop_requests_.push_back(&request);
					road_distances_.insert({ request_data.at("name").AsString(), &(request_data.at("road_distances").AsDict()) });
				}
			}
		}

		void JSONReader::FillStatRequests() {
			auto& root = commands_.GetRoot();
			const json::Array& stat_requests = root.AsDict().at("stat_requests").AsArray();

			for (const json::Node& request : stat_requests) {
				stat_requests_.push_back(&request);
			}
		}

		void JSONReader::FillRenderSettings() {
			auto& root = commands_.GetRoot();
			const json::Dict& render_settings = root.AsDict().at("render_settings").AsDict();

			for (auto& [setting, data] : render_settings) {
				render_settings_.insert({ setting, &data });
			}
		}
	} // ------------------ namespace json_reader ----------------

} // ------------------ namespace transport_catalogue ----------------
