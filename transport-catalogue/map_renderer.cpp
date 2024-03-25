#include <algorithm>
#include <vector>
#include <utility>

#include "map_renderer.h"
#include "svg.h"



/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace transport_catalogue {
	namespace detail {

		bool IsZero(double value) {
			return std::abs(value) < EPSILON;
		}

		svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
			return {
				(coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_
			};
		}
	}
	namespace map_renderer {

		svg::Circle MapRendererBase::CreateStopCircle() {
			svg::Circle circle;
			circle.SetRadius(settings_.stop_radius);
			circle.SetFillColor("white");
			return circle;
		}

		svg::Polyline MapRendererBase::CreateBusLine(int color_id) {
			svg::Polyline line;
			line.SetStrokeColor(settings_.color_palette[color_id % settings_.color_palette.size()]);
			line.SetFillColor("none");
			line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			line.SetStrokeWidth(settings_.line_width);
			line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			return line;
		}

		svg::Text MapRendererBase::CreateBusLabel(int color_id) {
			svg::Text text;
			text.SetFontFamily("Verdana");
			text.SetFontWeight("bold");
			text.SetFontSize(settings_.bus_label_font_size);
			text.SetOffset(settings_.bus_label_offset);
			text.SetFillColor(settings_.color_palette[color_id % settings_.color_palette.size()]);
			return text;
		}

		svg::Text MapRendererBase::CreateBusLabelUnderlayer() {
			svg::Text text;

			text.SetFontFamily("Verdana");
			text.SetFontSize(settings_.bus_label_font_size);
			text.SetOffset(settings_.bus_label_offset);
			text.SetFontWeight("bold");

			text.SetFillColor(settings_.underlayer_color);
			text.SetStrokeColor(settings_.underlayer_color);
			text.SetStrokeWidth(settings_.underlayer_width);

			text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			return text;
		}

		svg::Text MapRendererBase::CreateStopLabel() {
			svg::Text text;
			text.SetFillColor("black");
			text.SetFontFamily("Verdana");
			text.SetFontSize(settings_.stop_label_font_size);
			text.SetOffset(settings_.stop_label_offset);
			return text;
		}

		svg::Text MapRendererBase::CreateStopLabelUnderlayer() {
			svg::Text text;
			
			text.SetFontFamily("Verdana");
			text.SetFontSize(settings_.stop_label_font_size);
			text.SetOffset(settings_.stop_label_offset);

			text.SetFillColor(settings_.underlayer_color);
			text.SetStrokeColor(settings_.underlayer_color);
			text.SetStrokeWidth(settings_.underlayer_width);
			text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			return text;
		}

		MapRenderer::MapRenderer(
			MapRenderSettings&& settings,
			std::vector<Bus*> buses
			) :MapRendererBase(std::move(settings)) {
			InitNotEmptyBusesAndStops(buses);

			std::vector<geo::Coordinates> coords = GetAllCoords();
			detail::SphereProjector projector(coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);
			RenderBusLines(projector);
			RenderBusLabels(projector);
			RenderStops(projector);
			RenderStopLabels(projector);
		}

		void MapRenderer::InitNotEmptyBusesAndStops(std::vector<Bus*> buses) {
			std::sort(buses.begin(), buses.end(), [](const Bus* lhs, const Bus* rhs) {return std::lexicographical_compare(lhs->name_.begin(), lhs->name_.end(), rhs->name_.begin(), rhs->name_.end()); });
			for (auto* bus : buses) {
				if (!bus->stops_.empty()) {
					buses_.push_back(bus);
					for (auto* stop : bus->stops_) {
						stops_.insert(stop);
					}
				}
			}
		}

		std::vector<geo::Coordinates> MapRenderer::GetAllCoords() {
			std::vector<geo::Coordinates> coords;
			for (auto* stop : stops_) {
				coords.push_back(stop->coords_);
			}
			return coords;
		}


		void MapRenderer::RenderBusLines(detail::SphereProjector& projector) {
			for (size_t i = 0u; i < buses_.size(); i++) {
				svg::Polyline line = CreateBusLine(i);
				for (auto* stop : buses_[i]->stops_) {
					line.AddPoint(projector(stop->coords_));
				}
				if (buses_[i]->route_type_ == RouteType::DIRECT) {
					for (int j = buses_[i]->stops_.size() - 2; j >= 0; j--) {
						line.AddPoint(projector(buses_[i]->stops_[j]->coords_));
					}
				}
				Add(line);
			}
		}

		void MapRenderer::RenderStops(detail::SphereProjector& projector) {
			for (auto* stop : stops_) {
				svg::Circle circle = CreateStopCircle();
				circle.SetCenter(projector(stop->coords_));
				Add(circle);
			}
		}

		void MapRenderer::RenderBusLabels(detail::SphereProjector& projector) {
			for (size_t i = 0; i < buses_.size(); i++) {
				auto& stops = buses_[i]->stops_;
				Add(CreateBusLabelUnderlayer().SetPosition(projector(stops[0]->coords_)).SetData(buses_[i]->name_));
				Add(CreateBusLabel(i).SetPosition(projector(stops[0]->coords_)).SetData(buses_[i]->name_));
				if (buses_[i]->route_type_ != RouteType::ROUND && stops.front() != stops.back()) {
					Add(CreateBusLabelUnderlayer().SetPosition(projector(stops.back()->coords_)).SetData(buses_[i]->name_));
					Add(CreateBusLabel(i).SetPosition(projector(stops.back()->coords_)).SetData(buses_[i]->name_));
				}
			}
		}

		void MapRenderer::RenderStopLabels(detail::SphereProjector& projector) {
			for (auto* stop : stops_) {
				svg::Text text = CreateStopLabel();
				Add(CreateStopLabelUnderlayer().SetPosition(projector(stop->coords_)).SetData(stop->name_));
				Add(CreateStopLabel().SetPosition(projector(stop->coords_)).SetData(stop->name_));
			}
		}
	}
}

