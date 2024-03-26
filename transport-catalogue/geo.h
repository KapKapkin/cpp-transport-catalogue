#pragma once

#include <cmath>

namespace transport_catalogue {
    namespace geo {

        static constexpr int EARTH_RADIUS = 6371000;

        struct Coordinates {

            double lat;
            double lng;

            bool operator==(const Coordinates& other) const {
                return lat == other.lat && lng == other.lng;
            }
            bool operator!=(const Coordinates& other) const {
                return !(*this == other);
            }

        };
        double ComputeDistance(const Coordinates& from, const Coordinates& to);

    }  // ----------------- namespace geo -----------------
}// ----------------- namespace transport -----------------