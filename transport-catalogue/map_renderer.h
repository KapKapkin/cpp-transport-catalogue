#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <utility>
#include <string>
#include <set>

#include "domain.h"
#include "svg.h"


/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace transport_catalogue {
    namespace detail {
        // ---------------- SphereProjector  -------------------

        inline const double EPSILON = 1e-6;

        bool IsZero(double value);

        class SphereProjector {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
                : padding_(padding) //
            {
                // Если точки поверхности сферы не заданы, вычислять нечего
                if (points_begin == points_end) {
                    return;
                }

                // Находим точки с минимальной и максимальной долготой
                const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
                min_lon_ = left_it->lng;
                const double max_lon = right_it->lng;

                // Находим точки с минимальной и максимальной широтой
                const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
                const double min_lat = bottom_it->lat;
                max_lat_ = top_it->lat;

                // Вычисляем коэффициент масштабирования вдоль координаты x
                std::optional<double> width_zoom;
                if (!IsZero(max_lon - min_lon_)) {
                    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
                }

                // Вычисляем коэффициент масштабирования вдоль координаты y
                std::optional<double> height_zoom;
                if (!IsZero(max_lat_ - min_lat)) {
                    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
                }

                if (width_zoom && height_zoom) {
                    // Коэффициенты масштабирования по ширине и высоте ненулевые,
                    // берём минимальный из них
                    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
                }
                else if (width_zoom) {
                    // Коэффициент масштабирования по ширине ненулевой, используем его
                    zoom_coeff_ = *width_zoom;
                }
                else if (height_zoom) {
                    // Коэффициент масштабирования по высоте ненулевой, используем его
                    zoom_coeff_ = *height_zoom;
                }
            }

            // Проецирует широту и долготу в координаты внутри SVG-изображения
            svg::Point operator()(geo::Coordinates coords) const;

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };

    }

	// Локально всё отображается нормально, но после пуша в гит появляется лишний отступ. 
	namespace map_renderer { 
		// --------------- MapRenderer -------------

		struct MapRenderSettings {

			MapRenderSettings() = default;

			double width = 0.0;
			double height = 0.0;
			double padding = 0.0;

			double line_width = 0.0;
			double stop_radius = 0.0;

			int bus_label_font_size = 0;    
			svg::Point bus_label_offset{};
					
			double stop_label_font_size = 0.0;
			svg::Point stop_label_offset{};

			svg::Color underlayer_color{};
			double underlayer_width = 0.0;

			std::vector<svg::Color> color_palette;
		};


        class MapRendererBase {
        public:
            MapRendererBase(MapRenderSettings&& settings) : settings_(std::move(settings)) {}
            svg::Circle CreateStopCircle();
            svg::Polyline CreateBusLine(int color_id);
            svg::Text CreateBusLabel(int color_id);
            svg::Text CreateBusLabelUnderlayer();
            svg::Text CreateStopLabel();
            svg::Text CreateStopLabelUnderlayer();
        protected:
            const MapRenderSettings& settings_;
        };

        class MapRenderer : public svg::Document, private MapRendererBase {
        public:
            MapRenderer(MapRenderSettings&& settings, std::vector<Bus*> buses);
        private:
            std::vector<geo::Coordinates> GetAllCoords();

            void InitNotEmptyBusesAndStops(std::vector<Bus*> buses);

            void RenderBusLines(detail::SphereProjector& projector);
            void RenderStops(detail::SphereProjector& projector);
            void RenderBusLabels(detail::SphereProjector& projector);
            void RenderStopLabels(detail::SphereProjector& projector);

            struct StopPtrComparator {
                bool operator()(const Stop* lhs, const Stop* rhs) const { 
                    return std::lexicographical_compare(lhs->name_.begin(), lhs->name_.end(),
                                                        rhs->name_.begin(), rhs->name_.end());
                }
            };

            std::set<Stop*, StopPtrComparator> stops_;
            std::vector<Bus*> buses_;
        };
	}//--------------- map_renderer -------------

} //--------------- transport_catalogue -------------