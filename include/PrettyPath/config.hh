#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#pragma once

namespace Config {
struct {
    std::string nodes_filename;
    std::string edges_filename;
    std::string tarns_filename;
    std::string path_dir;
    float length_weight;
    float elevation_weight;
    float difficulty_weight;
    float cars_weight;
    float max_slope;
    float minimum_tarn_elevation;
    float maximum_tarn_elevation;
    float minimum_tarn_area;
    float minimum_latitude;
    float maximum_latitude;
    float minimum_longitude;
    float maximum_longitude;
    float min_dist_per_day;
} c = {};

inline void get_config(std::string config_filename) {
    std::ifstream config_file(config_filename);
    if (!config_file.is_open()) {
        std::cerr << "Could not open config file" << std::endl;
        exit(1);
    }

    nlohmann::json config;
    config_file >> config;

    c.nodes_filename = config["NODES_FILENAME"];
    c.edges_filename = config["EDGES_FILENAME"];
    c.tarns_filename = config["TARNS_FILENAME"];
    c.path_dir = config["PATH_DIR"];
    c.length_weight = config["LENGTH_WEIGHT"];
    c.elevation_weight = config["ELEVATION_WEIGHT"];
    c.difficulty_weight = config["DIFFICULTY_WEIGHT"];
    c.cars_weight= config["CARS_WEIGHT"];
    c.max_slope = config["MAX_SLOPE"];
    c.minimum_tarn_elevation = config["MINIMUM_TARN_ELEVATION"];
    c.maximum_tarn_elevation = config["MAXIMUM_TARN_ELEVATION"];
    c.minimum_tarn_area = config["MINIMUM_TARN_AREA"];
    c.minimum_latitude = config["MINIMUM_LATITUDE"];
    c.maximum_latitude = config["MAXIMUM_LATITUDE"];
    c.minimum_longitude = config["MINIMUM_LONGITUDE"];
    c.maximum_longitude = config["MAXIMUM_LONGITUDE"];
    c.min_dist_per_day = config["MIN_DIST_PER_DAY"];

    // check all values are present
    if (c.nodes_filename.empty()) {
        std::cerr << "Nodes filename not specified" << std::endl;
        exit(1);
    }
    if (c.edges_filename.empty()) {
        std::cerr << "Edges filename not specified" << std::endl;
        exit(1);
    }
    if (c.tarns_filename.empty()) {
        std::cerr << "Tarns filename not specified" << std::endl;
        exit(1);
    }
    if (c.path_dir.empty()) {
        std::cerr << "Path directory not specified" << std::endl;
        exit(1);
    }

    // check min and max values
    if (c.minimum_tarn_elevation > c.maximum_tarn_elevation) {
        std::cerr << "Minimum tarn elevation must be less than or equal to maximum tarn elevation" << std::endl;
        exit(1);
    }
    if (c.minimum_latitude > c.maximum_latitude) {
        std::cerr << "Minimum latitude must be less than or equal to maximum latitude" << std::endl;
        exit(1);
    }
    if (c.minimum_longitude > c.maximum_longitude) {
        std::cerr << "Minimum longitude must be less than or equal to maximum longitude" << std::endl;
        exit(1);
    }
}
} // namespace Config