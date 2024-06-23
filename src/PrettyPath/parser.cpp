#include "parser.hh"
#include <filesystem>
#include <nlohmann/json.hpp>

// Allocate memory for static variables
std::string Parser::m_nodes_filename;
std::string Parser::m_edges_filename;
double Parser::m_min_lat = std::numeric_limits<double>::max();
double Parser::m_max_lat = -std::numeric_limits<double>::max();
double Parser::m_min_lon = std::numeric_limits<double>::max();
double Parser::m_max_lon = -std::numeric_limits<double>::max();

Parser::Parser(std::string nodes_filename, std::string edges_filename) {
  m_nodes_filename = nodes_filename;
  m_edges_filename = edges_filename;
}

std::vector<node_id_t> Parser::parse_nodes(
    const std::string& edge_nodes_string) {
  std::vector<node_id_t> edge_nodes;

  std::stringstream ss(edge_nodes_string);
  std::string node_id;
  while (std::getline(ss, node_id, ',')) {
    edge_nodes.push_back(std::stol(node_id));
  }

  return edge_nodes;
}

MapData Parser::read_map_data(Graph& graph) {
  MapData map_data;  // Map node id to node

  std::cout << "Reading map data\n";

  std::ifstream nodes_file(m_nodes_filename);
  if (!nodes_file.is_open()) {
    std::cerr << "Error: Could not open file " << m_nodes_filename << std::endl;
    return map_data;
  }

  std::string line;
  std::getline(nodes_file, line);  // Skip the header
  while (std::getline(nodes_file, line)) {
    std::stringstream ss(line);
    std::string field;

    // Get the node id
    std::getline(ss, field, ',');
    long id = std::stol(field);

    // Get the latitude
    std::getline(ss, field, ',');
    double latitude = std::stod(field);
    if (latitude < m_min_lat) m_min_lat = latitude;
    if (latitude > m_max_lat) m_max_lat = latitude;

    // Get the longitude
    std::getline(ss, field, ',');
    double longitude = std::stod(field);
    if (longitude < m_min_lon) m_min_lon = longitude;
    if (longitude > m_max_lon) m_max_lon = longitude;

    // Get the elevation
    std::getline(ss, field);
    float elevation = std::stof(field);

    const Node* node = new Node(id, latitude, longitude, elevation);
    map_data.insert(std::make_pair(id, node));
  }

  nodes_file.close();

  std::ifstream edges_file(m_edges_filename);
  if (!edges_file.is_open()) {
    std::cerr << "Error: Could not open file " << m_edges_filename << std::endl;
    return map_data;
  }

  std::getline(edges_file, line);  // Skip the header
  while (std::getline(edges_file, line)) {
    std::stringstream ss(line);
    std::string field;

    // Get the edge id
    std::getline(ss, field, ',');
    long id = std::stol(field);

    // Get the osm_id
    std::getline(ss, field, ',');
    long osm_id = std::stol(field);

    // Get the source node id
    std::getline(ss, field, ',');
    long source_node_id = std::stol(field);

    // Get the target node id
    std::getline(ss, field, ',');
    long target_node_id = std::stol(field);

    // Get the length
    std::getline(ss, field, ',');
    double length = std::stod(field);

    // Get the slope
    std::getline(ss, field, ',');
    double slope = std::stod(field);

    // Get the difficulty
    std::getline(ss, field, ',');
    int difficulty = std::stoi(field);
    if (difficulty == -1) difficulty = 0;  // Normalize difficulty

    // Get the car
    std::getline(ss, field, ',');
    int car = std::stoi(field);
    if (car == -1) car = 0;  // Normalize car

    // Get geometry of the edge
    std::getline(ss, field);
    auto edge_nodes = parse_nodes(field);

    if (map_data.find(source_node_id) == map_data.end() ||
        map_data.find(target_node_id) == map_data.end()) {
      std::cerr << "Error: Edge node not found in map data for edge: " << osm_id
                << std::endl;
      continue;
    }
    if (source_node_id == target_node_id) {
      std::cerr << "Error: Source and Target node of edge: " << osm_id
                << " are the same!" << std::endl;
    }
    const Node* start_node = map_data[source_node_id];
    const Node* target_node = map_data[target_node_id];
    graph.add_edge(start_node, target_node, length, slope, car, osm_id,
                   edge_nodes);
  }

  edges_file.close();

  std::cout << "Done reading map data\n";
  std::cout << "Latitude range: " << m_min_lat << " -> " << m_max_lat
            << std::endl;
  std::cout << "Longitude range: " << m_min_lon << " -> " << m_max_lon
            << std::endl;
  return map_data;
}

std::vector<POIData> Parser::read_poi_data(const std::string& filename) {
  std::vector<POIData> poi_data;

  std::ifstream poi_file(filename);
  if (!poi_file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return poi_data;
  }

  std::string line;
  std::getline(poi_file, line);  // Skip the header
  while (std::getline(poi_file, line)) {
    std::stringstream ss(line);
    std::string field;

    // Get the osm_id
    std::getline(ss, field, ',');
    long osm_id = std::stol(field);

    // Get the name
    std::getline(ss, field, '"');
    std::getline(ss, field, '"');
    std::string name = field;
    std::getline(ss, field, ',');

    // Get the latitude
    std::getline(ss, field, ',');
    double latitude = std::stod(field);

    // Get the longitude
    std::getline(ss, field, ',');
    double longitude = std::stod(field);

    // Get the elevation
    std::getline(ss, field, ',');
    float elevation = std::stof(field);

    if (ss.eof()) {
      poi_data.push_back(POIData(name, latitude, longitude, osm_id, elevation));
      continue;
    }

    // Get the area (if available)
    std::getline(ss, field);
    unsigned long area = std::stoul(field);

    poi_data.push_back(
        POIData(name, latitude, longitude, osm_id, elevation, area));
  }

  poi_file.close();

  return poi_data;
}

std::vector<POIData> Parser::read_ordered_poi_data(
    const std::string& filename) {
  std::vector<POIData> poi_data;

  std::ifstream poi_file(filename);
  if (!poi_file.is_open()) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return poi_data;
  }

  nlohmann::json pois;
  poi_file >> pois;

  for (const auto& poi : pois) {
    std::string name = poi["name"];
    double latitude = poi["position"][0];
    double longitude = poi["position"][1];
    long osm_id = 0;
    float elevation = poi["elevation"];
    if (poi.contains("area")) {
      unsigned long area = poi["area"];
      poi_data.push_back(
          POIData(name, latitude, longitude, osm_id, elevation, area));
    } else {
      poi_data.push_back(POIData(name, latitude, longitude, osm_id, elevation));
    }
  }

  poi_file.close();

  return poi_data;
}

std::vector<std::pair<const long, const Node*>> Parser::path_to_node_list(
    const MapData& map_data, const Graph& graph,
    const std::vector<const Node*>& path) {
  std::vector<std::pair<const long, const Node*>> node_list;

  for (size_t i = 1; i < path.size(); i++) {
    const Node* node = path[i - 1];
    const Node* next_node = path[i];
    bool found_edge = false;
    if (node == nullptr) {
      std::cerr << "Error: Node is null" << std::endl;
      continue;
    }
    if (next_node == nullptr) {
      std::cerr << "Error: Next node is null" << std::endl;
      continue;
    }
    if (node == next_node) {
      continue;
    }

    // Find the edge between node and next_node
    for (const auto& edge_pair : graph.get_neighbours(node)) {
      if (edge_pair.first == next_node) {
        found_edge = true;
        Edge edge = edge_pair.second;
        // Check if the edge is in the correct direction
        edge.reverse_if_needed(node->get_id());
        auto edge_nodes = edge.get_edge_nodes();
        // Check the end node of the edge is the same as the next node
        if (next_node->get_id() != edge_nodes.back()) {
          std::cerr << "Error: Next node is not the end node of the edge"
                    << std::endl;
          continue;
        }
        if (next_node != path.back()) {  // If not the last node in the path
          edge_nodes.pop_back();  // Remove the end node from the edge (Prevent
                                  // duplicate nodes in the path)
        }
        const long edge_id = edge.get_osm_id();
        for (auto it = edge_nodes.begin(); it != edge_nodes.end(); it++) {
          const Node* edge_node = map_data.at(*it);
          node_list.push_back(std::make_pair(edge_id, edge_node));
        }
        break;
      }
    }
    if (!found_edge) {
      std::cerr << "Error: Edge not found between " << node->get_id() << " and "
                << next_node->get_id() << std::endl;
    }
  }

  return node_list;
}

void Parser::write_path_to_py(
    const std::vector<std::pair<const long, const Node*>>& node_list,
    const std::string& filename) {
  std::ofstream file(filename);
  long node_counter = 0;
  double total_length = 0;

  file << "id,lat,lon,length,elevation\n";
  file << std::fixed << std::setprecision(6);

  for (auto it = node_list.begin(); it != node_list.end(); it++) {
    const long edge_id = it->first;
    const Node* node = it->second;
    const Node* next_node =
        (it + 1 != node_list.end()) ? (it + 1)->second : nullptr;
    const auto location = node->get_location();
    double length = 0;
    if (next_node != nullptr) {
      const auto next_location = next_node->get_location();
      length = node->distance_to(next_location.first, next_location.second);
    }
    file << edge_id << "," << location.first << "," << location.second << ","
         << length << "," << node->get_elevation() << "\n";
    node_counter++;
    total_length += length;
  }

  file.close();
  std::cout << "Wrote " << node_counter << " nodes to " << filename
            << std::endl;
  std::cout << "Total length: " << total_length / 1000.f << " km" << std::endl;
}

std::ofstream Parser::write_gpx_header(const std::string& file_name) {
  std::ofstream file(file_name);
  file << std::fixed << std::setprecision(6);

  file << R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>)"
       << "\n";
  file
      << R"(<gpx xmlns="http://www.topografix.com/GPX/1/1" creator="PrettyPath" version="1.1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd">)"
      << "\n";

  return file;
}

void Parser::write_gpx_waypoint(std::ofstream& file, const std::string& name,
                                const double lat, const double lon,
                                const double elevation) {
  file << "<wpt lat=\"" << lat << "\" lon=\"" << lon << "\">\n";
  file << "<name>" << name << "</name>\n";
  file << "<ele>" << elevation << "</ele>\n";
  file << "</wpt>\n";
}

void Parser::write_gpx_track_segment(
    std::ofstream& file, const std::string& name,
    const std::vector<std::pair<const long, const Node*>>& node_list) {
  file << "<trk>\n";
  file << "<name>" << name << "</name>\n";
  file << "<trkseg>\n";
  for (auto it = node_list.begin(); it != node_list.end(); it++) {
    const Node* node = it->second;
    if (node == nullptr) {
      std::cerr << "Error: Node is null" << std::endl;
      continue;
    }
    const auto location = node->get_location();

    file << "<trkpt lat=\"" << location.first << "\" lon=\"" << location.second
         << "\">\n";
    file << "<ele>" << node->get_elevation() << "</ele>\n";
    file << "</trkpt>\n";
  }
  file << "</trkseg>\n";
  file << "</trk>\n";
}

void Parser::write_gpx_footer(std::ofstream& file) {
  file << "</gpx>\n";
  file.close();
}

void Parser::write_paths(
    const MapData& map_data, const Graph& graph,
    const std::pair<std::vector<std::pair<const POIData, size_t>>,
                    std::vector<const Node*>>& poi_path,
    const std::string& file_dir, const std::string& gpx_filename) {
  auto pois = poi_path.first;
  auto path = poi_path.second;
  size_t path_start = 0;
  size_t edges_written = 0;

  for (const auto& entry : std::filesystem::directory_iterator(file_dir)) {
    std::filesystem::remove(entry);
  }

  std::ofstream gpx = write_gpx_header(file_dir + gpx_filename);

  for (size_t i = 0; i < pois.size() - 1; i++) {
    auto path_length = pois[i].second;
    edges_written += path_length;
    auto start_poi = pois[i].first;
    auto end_poi = pois[(i + 1) % pois.size()].first;
    std::cout << "Writing path from " << start_poi.name << " to "
              << end_poi.name << " with " << path_length << " edges"
              << std::endl;
    std::string name = start_poi.name_without_spaces() + "_to_" +
                       end_poi.name_without_spaces();
    std::string filename = file_dir + name + ".csv";
    std::vector<const Node*> sub_path(path.begin() + path_start,
                                      path.begin() + path_start + path_length);
    path_start += path_length;
    auto node_list = path_to_node_list(map_data, graph, sub_path);

    write_gpx_waypoint(gpx, start_poi.name, start_poi.latitude,
                       start_poi.longitude, start_poi.elevation);

    write_gpx_track_segment(gpx, name, node_list);
    write_path_to_py(node_list, filename);
  }
  write_gpx_waypoint(gpx, pois.back().first.name, pois.back().first.latitude,
                     pois.back().first.longitude,
                     pois.back().first.elevation);
  write_gpx_footer(gpx);
}

void Parser::clean_map_data(MapData& map_data) {
  for (auto& pair : map_data) {
    delete pair.second;
  }
  map_data.clear();
}