#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/haversine.hpp>
#include <iostream>
#include <fstream>
#include <gdal_priv.h>
#pragma once

namespace osmparser {
class Handler : public osmium::handler::Handler {
public:
    Handler(std::string data_filename, std::string edges_filename, std::string nodes_filename) : m_elevation_filename(data_filename), m_edges_file(edges_filename), m_nodes_file(nodes_filename) {
        std::cout << "Reading elevation data..." << std::endl;
        read_elevation_data();
        std::cout << "Reading OSM data...\n";
    }
    ~Handler();

    void node(const osmium::Node& node);
    void way(const osmium::Way& way);

private:
    void read_elevation_data();
    float get_elevation(const double latitude, const double longitude);
    bool is_walkable(const osmium::TagList& tags);
    int get_cars(const osmium::TagList& tags);
    int get_difficulty(const osmium::TagList& tags);
    void write_nodes_file();
    void write_edges_file();

private:
    struct NodeData {
        NodeData() : location(osmium::Location()), ways(0), elevation(0) {}
        NodeData(osmium::Location location, unsigned int ways, float elevation) : location(location), ways(ways), elevation(elevation) {}
        osmium::Location location;
        unsigned int ways; // Number of ways that pass through this node
        float elevation;
    };
    struct WayData {
        WayData() : walkable(false), cars(-1), difficulty(-1), nodes(std::vector<osmium::object_id_type>()) {}
        WayData(bool walkable, int cars, int difficulty, std::vector<osmium::object_id_type> nodes) : walkable(walkable), cars(cars), difficulty(difficulty), nodes(nodes) {}
        bool walkable;
        int cars, difficulty;
        std::vector<osmium::object_id_type> nodes;
    };

private:
    std::string m_elevation_filename;
    GDALDataset* m_poDataset;
    std::ofstream m_edges_file;
    std::ofstream m_nodes_file;
    std::unordered_map<osmium::object_id_type, NodeData> m_nodes;
    long m_node_counter = 0;
    std::unordered_map<osmium::object_id_type, WayData> m_ways;
    long m_edge_counter = 0;
    double m_ele_min_lon, m_map_min_lon;
    double m_ele_max_lon, m_map_max_lon;
    double m_ele_min_lat, m_map_min_lat;
    double m_ele_max_lat, m_map_max_lat;
};
} // namespace osmparser