#pragma once

#include <unordered_map>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_graph {

	using namespace transport_catalogue;

	using TransportTime = double;
	static constexpr double TO_MINUTES = (3.6 / 60.0);

	struct VertexIdLoop {
		graph::VertexId id;
		graph::VertexId transfer_id;
	};

	struct TransportGraphData {
		const domain::Stop* stop_from;
		const domain::Stop* stop_to;
		const domain::Bus* bus;
		int stop_count;
		double time;
	};

	using EdgesData = std::unordered_map<graph::VertexId, std::unordered_map<graph::VertexId, TransportGraphData>>;

	class TransportGraph {
	public:
		explicit TransportGraph(const TransportCatalogue& catalogue) 
			:graph_(catalogue.GetStops().size() * 2) {
			SetVertex(catalogue);
			CreateDiagonalEdges(catalogue);
			CreateGraph(catalogue);	
		}

		const graph::DirectedWeightedGraph<TransportTime>& GetGraph() const {
			return graph_;
		}

		const std::unordered_map<graph::EdgeId, TransportGraphData>& GetEdgeIdToGraphData() const {
			return edge_id_to_graph_data_;
		}

		const std::unordered_map<const domain::Stop*, VertexIdLoop>& GetStopToVertexId() const {
			return stop_to_vertex_id_;
		}

	private:

		void SetVertex(const TransportCatalogue& catalogue);
		void CreateDiagonalEdges(const TransportCatalogue& catalogue);
		void CreateGraph(const TransportCatalogue& catalogue);

		template <typename It>
		std::vector<TransportGraphData> CreateTransportGraphData(const ranges::BusRange<It>& bus_range, const TransportCatalogue& catalogue);

		void CreateEdges(EdgesData& edges, std::vector<TransportGraphData>&& data);
		void AddEdgesToGraph(EdgesData& edges);

		std::unordered_map<graph::EdgeId, TransportGraphData> edge_id_to_graph_data_{};
		std::unordered_map<const domain::Stop*, VertexIdLoop> stop_to_vertex_id_{};
		graph::DirectedWeightedGraph<TransportTime> graph_{};
	};

	class TransportRouter {
	public:

		struct TransportRouterData {
			std::vector<TransportGraphData> route{};
			TransportTime time{};
		};

		TransportRouter(const TransportGraph& graph) :transport_graph_(graph), router_(graph.GetGraph())
		{}

		std::optional<TransportRouter::TransportRouterData> GetRoute(const domain::Stop* from, const domain::Stop* to) const;

	private:
		TransportGraph transport_graph_;
		graph::Router<TransportTime> router_;
	};

	template <typename It>
	inline std::vector<TransportGraphData> TransportGraph::CreateTransportGraphData(const ranges::BusRange<It>& bus_range, const TransportCatalogue& catalogue) {

		const double bus_velocity = catalogue.GetRoutingSettings().bus_velocity;

		std::vector<TransportGraphData> data;

		for (auto it_from = bus_range.begin(); it_from != bus_range.end(); ++it_from) {
			const domain::Stop* stop_from = *it_from;
			const domain::Stop* previous_stop = stop_from;

			double full_distance = 0.0;
			int stop_count = 0;

			for (auto it_to = it_from + 1; it_to != bus_range.end(); ++it_to) {
				const domain::Stop* stop_to = *it_to;

				if (stop_from != stop_to) {
					full_distance += catalogue.GetDistance(previous_stop->name_, stop_to->name_ );
					stop_count++;

					data.push_back({ stop_from, stop_to, bus_range.GetPtr(), stop_count, (full_distance / bus_velocity) * TO_MINUTES });
				}

				previous_stop = stop_to;
			}
		}

		return data;
	}

}