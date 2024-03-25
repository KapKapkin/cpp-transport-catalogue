#include "domain.h"

namespace transport_catalogue {
	RouteType IntToRouteType(int i) {
		switch (i)
		{
		case 0:
			return RouteType::DIRECT;
		case 1:
			return RouteType::ROUND;
		default:
			return RouteType::UNKNOWN;
		}

	}
}