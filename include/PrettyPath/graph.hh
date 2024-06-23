#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "config.hh"
#include "utils.hh"
#pragma once

using node_id_t = long;

class Node {
 public:
  Node(size_t id, double latitude, double longitude, float elevation = 0)
      : m_id(id),
        m_latitude(latitude),
        m_longitude(longitude),
        m_elevation(elevation) {}

  node_id_t get_id() const { return m_id; }

  float get_elevation() const { return m_elevation; }

  std::pair<double, double> get_location() const {
    return std::make_pair(m_latitude, m_longitude);
  }

  bool operator==(const Node& other) const { return m_id == other.get_id(); }

  double distance_to(double lat, double lon) const {
    return utils::haversine_distance(m_latitude, m_longitude, lat, lon);
  }

 private:
  node_id_t m_id;
  double m_latitude, m_longitude;
  float m_elevation;
};

class Edge {
 public:
  Edge(double length, double slope, int cars = 0, long osm_id = 0,
       std::vector<long> edge_nodes = {})
      : m_length(length),
        m_slope(slope),
        m_cars(cars),
        m_osm_id(osm_id),
        m_edge_nodes(edge_nodes) {}

  long get_osm_id() const { return m_osm_id; }

  int get_difficulty() const { return m_difficulty; }

  int get_cars() const { return m_cars; }

  void reverse_if_needed(node_id_t desired_source_id) {
    if (desired_source_id == m_edge_nodes.front()) {
      return;
    }
    std::reverse(m_edge_nodes.begin(), m_edge_nodes.end());
  }

  std::vector<node_id_t> get_edge_nodes() const { return m_edge_nodes; }

  double cost() const {
    const double cost =
        Config::c.length_weight * m_length +
        Config::c.elevation_weight * (m_slope + 3) +  // TODO fix
        Config::c.cars_weight * m_cars +
        Config::c.difficulty_weight * m_difficulty;
    return cost;
  }

  double elevation_change() const { return m_length * m_slope; }

 private:
  long m_osm_id, m_source_id, m_target_id;
  double m_length, m_slope;
  int m_cars, m_difficulty;
  std::vector<node_id_t> m_edge_nodes;
};

// Store the graph as an adjacency list
// The key is the node and the value is a vector of connecting nodes (node,
// edge)
using graph_t =
    std::unordered_map<const Node*,
                       std::vector<std::pair<const Node*, const Edge>>>;

class Graph {
 public:
  Graph() = default;

  void add_edge(const Node* node1, const Node* node2, const double length,
                const double slope, const int cars = 0, const long osm_id = 0,
                const std::vector<node_id_t> edge_nodes = {});
  std::vector<const Node*> get_nodes() const;
  void apply_to_nodes(std::function<void(const Node*)> func) const;
  std::vector<std::pair<const Node*, const Edge>> get_neighbours(
      const Node* node) const;
  std::pair<const Node*, double> find_closest_node(
      const double latitude, const double longitude) const;
  void print_graph_info() const;

 private:
  graph_t m_graph;
};

struct POIData {
  POIData() : name(""), latitude(0), longitude(0), osm_id(0), elevation(0) {}
  POIData(std::string name, double latitude, double longitude, long osm_id,
          float elevation, unsigned long area = 0)
      : name(name),
        latitude(latitude),
        longitude(longitude),
        osm_id(osm_id),
        elevation(elevation),
        area(area) {}
  std::string name_without_spaces() {
    std::string name_without_spaces = name;
    std::replace(name_without_spaces.begin(), name_without_spaces.end(), ' ',
                 '_');
    return name_without_spaces;
  }
  std::string name;
  double latitude, longitude;
  long osm_id;
  float elevation;
  const Node* best_node = nullptr;
  unsigned long area; // Only for tarns
};