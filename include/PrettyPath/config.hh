#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#pragma once

namespace Config {
struct config_t {
  // Filenames
  std::string nodes_filename;
  std::string edges_filename;
  std::string tarns_filename;
  std::string output_dir;
  std::string gpx_filename;
  // Cost
  float length_weight;
  float elevation_weight;
  float difficulty_weight;
  float cars_weight;
  // Tarn Constraints
  float min_tarn_elevation;
  float max_tarn_elevation;
  float min_tarn_area;
  float max_tarn_area;
  std::vector<std::string> tarn_blacklist;
  bool use_ordered_tarns = false;
  // Path Constraints
  float max_path_length;  // TODO unused
  float min_path_length;
  float max_elevation_gain;  // TODO unused
  int max_difficulty;
  int max_cars;
  std::pair<double, double> start_location;
  // Map Constraints
  double min_latitude;
  double max_latitude;
  double min_longitude;
  double max_longitude;
};

extern config_t c;

inline void get_config(std::string config_filename) {
  std::ifstream config_file(config_filename);
  if (!config_file.is_open()) {
    std::cerr << "Could not open config file" << std::endl;
    exit(1);
  }

  nlohmann::json config;
  config_file >> config;

  nlohmann::json filenames = config["filenames"];
  c.nodes_filename = filenames["map_nodes"];
  c.edges_filename = filenames["map_edges"];
  c.tarns_filename = filenames["map_tarns"];
  c.output_dir = filenames["output_dir"];
  c.gpx_filename = filenames["gpx"];
  nlohmann::json weights = config["path_cost"];
  if (weights.find("length_weight") == weights.end() ||
      weights.find("elevation_weight") == weights.end() ||
      weights.find("difficulty_weight") == weights.end() ||
      weights.find("cars_weight") == weights.end()) {
    std::cerr << "Path cost weights not specified" << std::endl;
    exit(1);
  }
  c.length_weight = weights["length_weight"];
  c.elevation_weight = weights["elevation_weight"];
  c.difficulty_weight = weights["difficulty_weight"];
  c.cars_weight = weights["cars_weight"];
  nlohmann::json tarn_constraints = config["tarn_constraints"];
  if (tarn_constraints.find("use_ordered_tarns") != tarn_constraints.end())
    c.use_ordered_tarns = tarn_constraints["use_ordered_tarns"];
  else {
    c.min_tarn_elevation = tarn_constraints["min_elevation"];
    c.max_tarn_elevation = tarn_constraints["max_elevation"];
    c.min_tarn_area = tarn_constraints["min_area"];
    c.max_tarn_area = tarn_constraints["max_area"];
    if (tarn_constraints.find("blacklist") != tarn_constraints.end()) {
      for (nlohmann::json::iterator it = tarn_constraints["blacklist"].begin();
           it != tarn_constraints["blacklist"].end(); ++it) {
        c.tarn_blacklist.push_back(it.value());
      }
    }
  }
  nlohmann::json path_constraints = config["path_constraints"];
  // if(path_constraints != nullptr)
  c.max_path_length = path_constraints["max_length"];
  c.min_path_length = path_constraints["min_length"];
  c.max_elevation_gain = path_constraints["max_elevation"];
  c.max_difficulty = path_constraints["max_difficulty"];
  c.max_cars = path_constraints["max_cars"];
  if (path_constraints.find("start_location") != path_constraints.end())
    c.start_location =
        std::make_pair(path_constraints["start_location"]["latitude"],
                       path_constraints["start_location"]["longitude"]);
  nlohmann::json map_constraints = config["map_constraints"];
  c.min_latitude = map_constraints["min_latitude"];
  c.max_latitude = map_constraints["max_latitude"];
  c.min_longitude = map_constraints["min_longitude"];
  c.max_longitude = map_constraints["max_longitude"];
}

inline bool check_config() {
  if (c.nodes_filename.empty()) {
    std::cerr << "Nodes filename not specified" << std::endl;
    return false;
  }
  if (c.edges_filename.empty()) {
    std::cerr << "Edges filename not specified" << std::endl;
    return false;
  }
  if (c.tarns_filename.empty()) {
    std::cerr << "Tarns filename not specified" << std::endl;
    return false;
  }
  if (c.output_dir.empty()) {
    std::cerr << "Path directory not specified" << std::endl;
    return false;
  }
  if (c.gpx_filename.empty()) {
    std::cerr << "GPX filename not specified" << std::endl;
    return false;
  }
  if (c.min_path_length > c.max_path_length) {
    std::cerr << "Minimum path length must be less than or equal to maximum "
                 "path length"
              << std::endl;
    return false;
  }
  if (c.min_tarn_elevation > c.max_tarn_elevation) {
    std::cerr << "Minimum tarn elevation must be less than or equal to maximum "
                 "tarn elevation"
              << std::endl;
    return false;
  }
  if (c.min_latitude > c.max_latitude) {
    std::cerr
        << "Minimum latitude must be less than or equal to maximum latitude"
        << std::endl;
    return false;
  }
  if (c.min_longitude > c.max_longitude) {
    std::cerr
        << "Minimum longitude must be less than or equal to maximum longitude"
        << std::endl;
    return false;
  }
  return true;
}

inline void print_config() {
  std::cout << "Configuration:" << std::endl;
  std::cout << "\tFilenames:" << std::endl;
  std::cout << "\t\tNodes filename: " << c.nodes_filename << std::endl;
  std::cout << "\t\tEdges filename: " << c.edges_filename << std::endl;
  std::cout << "\t\tTarns filename: " << c.tarns_filename << std::endl;
  std::cout << "\t\tOutput directory: " << c.output_dir << std::endl;
  std::cout << "\t\tGPX filename: " << c.gpx_filename << std::endl;
  std::cout << "\tPath cost weights:" << std::endl;
  std::cout << "\t\tLength weight: " << c.length_weight << std::endl;
  std::cout << "\t\tElevation weight: " << c.elevation_weight << std::endl;
  std::cout << "\t\tDifficulty weight: " << c.difficulty_weight << std::endl;
  std::cout << "\t\tCars weight: " << c.cars_weight << std::endl;
  std::cout << "\tTarn constraints:" << std::endl;
  std::cout << "\t\tMinimum tarn elevation: " << c.min_tarn_elevation
            << std::endl;
  std::cout << "\t\tMaximum tarn elevation: " << c.max_tarn_elevation
            << std::endl;
  std::cout << "\t\tMinimum tarn area: " << c.min_tarn_area << std::endl;
  std::cout << "\t\tMaximum tarn area: " << c.max_tarn_area << std::endl;
  std::cout << "\t\tTarn blacklist:" << std::endl;
  for (auto tarn : c.tarn_blacklist) {
    std::cout << "\t\t\t" << tarn << std::endl;
  }
  std::cout << "\tPath constraints:" << std::endl;
  std::cout << "\t\tMaximum path length: " << c.max_path_length << std::endl;
  std::cout << "\t\tMinimum path length: " << c.min_path_length << std::endl;
  std::cout << "\t\tMaximum elevation gain: " << c.max_elevation_gain
            << std::endl;
  std::cout << "\t\tMaximum difficulty: " << c.max_difficulty << std::endl;
  std::cout << "\t\tMaximum cars: " << c.max_cars << std::endl;
  std::cout << "\t\tStart location: (" << c.start_location.first << ", "
            << c.start_location.second << ")" << std::endl;
  std::cout << "\tMap constraints:" << std::endl;
  std::cout << "\t\tMinimum latitude: " << c.min_latitude << std::endl;
  std::cout << "\t\tMaximum latitude: " << c.max_latitude << std::endl;
  std::cout << "\t\tMinimum longitude: " << c.min_longitude << std::endl;
  std::cout << "\t\tMaximum longitude: " << c.max_longitude << std::endl;
}
}  // namespace Config