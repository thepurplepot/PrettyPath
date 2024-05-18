#include "graph.hh"

void Graph::add_edge(const Node* node1, const Node* node2, const double length,
                     const double slope, const int cars, const long osm_id,
                     const std::vector<node_id_t> edge_nodes) {
  const Edge edge(length, slope, cars, osm_id, edge_nodes);
  if (m_graph.find(node1) == m_graph.end()) {
    m_graph[node1] = std::vector<std::pair<const Node*, const Edge>>();
  }
  if (m_graph.find(node2) == m_graph.end()) {
    m_graph[node2] = std::vector<std::pair<const Node*, const Edge>>();
  }
  m_graph[node1].push_back(std::make_pair(node2, edge));
  m_graph[node2].push_back(std::make_pair(node1, edge));
}

std::vector<const Node*> Graph::get_nodes() const {
  std::vector<const Node*> nodes;
  for (const auto& pair : m_graph) {
    nodes.push_back(pair.first);
  }
  return nodes;
}

void Graph::apply_to_nodes(std::function<void(const Node*)> func) const {
  for (const auto& pair : m_graph) {
    func(pair.first);
  }
}

std::vector<std::pair<const Node*, const Edge>> Graph::get_neighbours(
    const Node* node) const {
  return m_graph.at(node);
}

std::pair<const Node*, double> Graph::find_closest_node(
    const double latitude, const double longitude) const {
  const Node* closest_node = nullptr;
  double min_distance = std::numeric_limits<double>::max();

  for (auto& pair : m_graph) {
    const Node* node = pair.first;
    const double distance = node->distance_to(latitude, longitude);
    if (distance < min_distance) {
      min_distance = distance;
      closest_node = node;
    }
  }

  return std::make_pair(closest_node, min_distance);
}

void Graph::print_graph_info() const {
  size_t num_nodes = m_graph.size();
  size_t num_edges = 0;
  std::map<node_id_t, node_id_t> num_edges_per_node;
  for (const auto& pair : m_graph) {
    num_edges += pair.second.size();
    const Node* node = pair.first;
    if (num_edges_per_node.find(pair.second.size()) ==
        num_edges_per_node.end()) {
      num_edges_per_node[pair.second.size()] = 0;
    }
    num_edges_per_node[pair.second.size()] += 1;
  }
  std::cout << "Graph info: " << num_nodes << " nodes, " << num_edges
            << " edges" << std::endl;
  for (const auto& pair : num_edges_per_node) {
    std::cout << pair.second << " nodes have " << pair.first << " edges"
              << std::endl;
  }
}