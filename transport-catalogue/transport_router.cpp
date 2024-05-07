#include "transport_router.h"
#include "transport_catalogue.h"


using namespace transport_graph;



void TransportGraph::SetVertex(const TransportCatalogue& catalogue) {
	const auto& stops = catalogue.GetStops();
	graph::VertexId id{};
	for (const auto& [stopname, stop_ptr] : stops) {
		VertexIdLoop vertex_id{ id, id + 1 };
		stop_to_vertex_id_.insert({ stop_ptr, vertex_id });
		id += 2;
	}
}

void TransportGraph::CreateDiagonalEdges(const TransportCatalogue& catalogue) {
	const auto time = catalogue.GetRoutingSettings().bus_wait_time;
	for (const auto [stop_ptr, vertex_id] : stop_to_vertex_id_ ) {
		graph::EdgeId id = graph_.AddEdge({ vertex_id.transfer_id, vertex_id.id, time });
		edge_id_to_graph_data_.insert({ id, TransportGraphData{ stop_ptr, stop_ptr, nullptr, 0, time } });
	}
}

void  TransportGraph::CreateGraph(const TransportCatalogue& catalogue) {
	const auto& buses = catalogue.GetBuses();
	EdgesData edges;
	for (const auto& [busname, bus_ptr] : buses) {
		CreateEdges(edges, CreateTransportGraphData(ranges::AsBusRangeDirect(bus_ptr), catalogue));

		if (bus_ptr->route_type_ == domain::RouteType::DIRECT) {
			CreateEdges(edges, CreateTransportGraphData(ranges::AsBusRangeReversed(bus_ptr), catalogue));
		}
	}
	AddEdgesToGraph(edges);
}

void TransportGraph::CreateEdges(EdgesData& edges, std::vector<TransportGraphData>&& data) {
	for (TransportGraphData& data_i : data) {
		graph::VertexId from = stop_to_vertex_id_.at(data_i.stop_from).id;
		graph::VertexId to = stop_to_vertex_id_.at(data_i.stop_to).transfer_id;

		if (edges.count(from) > 0 && edges.at(from).count(to) > 0) {
			if (edges.at(from).at(to).time > data_i.time) {
				edges.at(from).at(to) = std::move(data_i);
			}
		}
		else {
			edges[from].emplace(to, std::move(data_i));
		}
	}
}

void TransportGraph::AddEdgesToGraph(EdgesData& edges) {
	for (auto& [from, to_map] : edges) {
		for (auto& [to, data_i] : to_map) {
			graph::EdgeId id = graph_.AddEdge({ from, to, data_i.time });
			edge_id_to_graph_data_.emplace(id, std::move(data_i));
		}
	}
}

std::optional<TransportRouter::TransportRouterData> TransportRouter::GetRoute(const domain::Stop* from, const domain::Stop* to) const {
	const auto& stop_to_vertex_id = transport_graph_.GetStopToVertexId();
	auto route = router_.BuildRoute(stop_to_vertex_id.at(from).transfer_id, stop_to_vertex_id.at(to).transfer_id);
	if (route) {
		TransportRouterData output_data;
		output_data.time = (*route).weight;

		const auto& edge_id_to_graph_data = transport_graph_.GetEdgeIdToGraphData();
		for (graph::EdgeId id : (*route).edges) {
			output_data.route.push_back(edge_id_to_graph_data.at(id));
		}
		return output_data;
	}
	return std::nullopt;
}