#include "graph.hh"
#include <string>
#include <sstream>
#include <fstream>
#pragma once

using MapData = std::unordered_map<long, const Node*>; // Map node id to node

class Parser {
public:
    Parser(std::string nodes_filename, std::string edges_filename);

    static std::vector<node_id_t> parse_nodes(const std::string& edge_nodes_string);
    static MapData read_map_data(Graph& graph);
    static std::vector<const TarnData> read_tarn_data(const std::string& filename);
    static void write_path_to_py(const MapData& map_data, const Graph& graph, const std::vector<const Node*>& path, const std::string& filename);
    static void write_tarn_paths(const MapData& map_data, const Graph& graph, const std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>>& tarns_path, const std::string& file_dir);
    static void clean_map_data(MapData& map_data);

private:
    static std::string m_nodes_filename;
    static std::string m_edges_filename;
    static double m_min_lat, m_max_lat, m_min_lon, m_max_lon;
};

