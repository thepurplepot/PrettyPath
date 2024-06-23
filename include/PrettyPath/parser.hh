#include <fstream>
#include <sstream>
#include <string>
#include "graph.hh"
#pragma once

using MapData = std::unordered_map<long, const Node*>;  // Map node id to node

class Parser {
 public:
  Parser(std::string nodes_filename, std::string edges_filename);

  static std::vector<node_id_t> parse_nodes(
      const std::string& edge_nodes_string);
  static MapData read_map_data(Graph& graph);
  static std::vector<POIData> read_poi_data(const std::string& filename);
  static std::vector<POIData> read_ordered_poi_data(
      const std::string& filename);
  static std::vector<std::pair<const long, const Node*>> path_to_node_list(
      const MapData& map_data, const Graph& graph,
      const std::vector<const Node*>& path);
  static void write_path_to_py(
      const std::vector<std::pair<const long, const Node*>>& node_list,
      const std::string& filename);
  static std::ofstream write_gpx_header(const std::string& file_name);
  static void write_gpx_waypoint(std::ofstream& file, const std::string& name,
                                 const double lat, const double lon,
                                 const double elevation);
  static void write_gpx_track_segment(
      std::ofstream& file, const std::string& name,
      const std::vector<std::pair<const long, const Node*>>& node_list);
  static void write_gpx_footer(std::ofstream& file);
  //   static void write_path_to_gpx(
  //       const std::vector<std::pair<const long, const Node*>>&
  //       full_node_list, const std::vector<std::tuple<std::string, const
  //       double, const double>>&
  //           tarn_nodes,
  //       const std::string& filename);
  static void write_paths(
      const MapData& map_data, const Graph& graph,
      const std::pair<std::vector<std::pair<const POIData, size_t>>,
                      std::vector<const Node*>>& tarns_path,
      const std::string& file_dir, const std::string& gpx_filename);
  static void clean_map_data(MapData& map_data);

 private:
  static std::string m_nodes_filename;
  static std::string m_edges_filename;
  static double m_min_lat, m_max_lat, m_min_lon, m_max_lon;
};
