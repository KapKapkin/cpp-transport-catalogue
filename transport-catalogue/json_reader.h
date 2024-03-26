#pragma once

#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "json.h"
#include "transport_catalogue.h"


namespace transport_catalogue {
	namespace json_reader {

		// -------------- JSONReader --------------

		class JSONReader {
		public:

			explicit JSONReader(std::istream& in);

			const std::vector<const json::Node*>& GetStatRequests() const;

			const std::vector<const json::Node*>& GetBusRequests() const;

			const std::vector<const json::Node*>& GetStopRequests() const;

			const std::unordered_map<std::string_view, const json::Node*>& GetRenderSettings() const;

			const std::unordered_map<std::string_view, const json::Dict*>& GetRoadDistances() const;

		private:

			void FillBusRequests();
			void FillStopRequests();
			void FillStatRequests();
			void FillRenderSettings();

			json::Document commands_;
			std::vector<const json::Node*> stat_requests_;
			std::vector<const json::Node*> bus_requests_;
			std::vector<const json::Node*> stop_requests_;
			std::unordered_map<std::string_view, const json::Node*> render_settings_;
			std::unordered_map<std::string_view, const json::Dict*> road_distances_;
		};
	} // ------------------ namespace json_reader ----------------

} // ------------------ namespace transport_catalogue ----------------

