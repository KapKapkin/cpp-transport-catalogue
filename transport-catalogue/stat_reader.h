#pragma once

#include <iosfwd>
#include <string_view>
#include <map>

#include "transport_catalogue.h"



namespace transport_catalogue {
	namespace statisctics {

		void ParseInputAndPrintStat(std::istream& in, std::ostream& out, TransportCatalogue& catalogue);

		static std::pair<std::string_view, std::string_view> ParseRequest(std::string_view request);

		void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
			std::ostream& output);

	}

}


